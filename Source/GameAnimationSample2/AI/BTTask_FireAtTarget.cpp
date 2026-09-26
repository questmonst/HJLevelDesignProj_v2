// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_FireAtTarget.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "AIController.h"
#include "WeaponBase.h"

UBTTask_FireAtTarget::UBTTask_FireAtTarget()
{
	NodeName     = TEXT("Fire At Target");
	bNotifyTick  = true;
	bCreateNodeInstance = false;

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FireAtTarget, TargetKey), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FireAtTarget, TargetKey));
}

void UBTTask_FireAtTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (const UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UBTTask_FireAtTarget::GetInstanceMemorySize() const
{
	return sizeof(FBTFireAtTargetMemory);
}

FString UBTTask_FireAtTarget::GetStaticDescription() const
{
	const FString Timing = bUseEnemyData
		? FString::Printf(TEXT("EnemyData %s 패턴"), *UEnum::GetDisplayValueAsText(Pattern).ToString())
		: FString::Printf(TEXT("%.1f초 사격 / %.1f초 휴식"), FireDuration, RestDuration);
	return FString::Printf(TEXT("%s에 %s (±%.1f)"), *TargetKey.SelectedKeyName.ToString(), *Timing, DurationDeviation);
}

EBTNodeResult::Type UBTTask_FireAtTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	AEnemyCharacter* Enemy = AICon ? Cast<AEnemyCharacter>(AICon->GetPawn()) : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Enemy || !BB) return EBTNodeResult::Failed;

	AWeaponBase* Weapon = Enemy->GetEnemyWeapon();
	if (!Weapon) return EBTNodeResult::Failed;

	// 쓰러져 있거나(날아가는 중 포함) 일어나는 중에는 못 쏜다. 다 일어나면 다시 이 태스크로 돌아온다
	if (Enemy->IsIncapacitated()) return EBTNodeResult::Failed;

	// 탄창이 비었으면 쏠 수 없다 — 재장전 분기가 곧바로 잡도록 키를 세우고 실패
	if (!Weapon->CanFire())
	{
		if (!Weapon->IsReloading()) BB->SetValueAsBool(AEnemyCharacter::BBKey_bNeedsReload, true);
		return EBTNodeResult::Failed;
	}

	// 조준점: 액터 키면 액터 위치(+시야 확인), 벡터 키면 마지막 발견 위치(제압 사격)
	const bool bVectorKey = TargetKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass();
	FVector AimLocation;
	if (bVectorKey)
	{
		AimLocation = BB->GetValueAsVector(TargetKey.SelectedKeyName);
		if (!FAISystem::IsValidLocation(AimLocation)) return EBTNodeResult::Failed;
	}
	else
	{
		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
		if (!Target) return EBTNodeResult::Failed;
		if (bRequireLineOfSight && !AICon->LineOfSightTo(Target))
		{
			return bFailWhenOutOfRange ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;
		}
		AimLocation = Target->GetActorLocation();
	}

	// 사거리 밖이면 쏘지 않는다 — 실패시키면 상위 Sequence가 끊겨 다시 접근 단계로 돌아간다
	if (FVector::Dist(Enemy->GetActorLocation(), AimLocation) > Enemy->GetAttackRange())
	{
		return bFailWhenOutOfRange ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;
	}

	float Burst = FireDuration;
	float Rest  = RestDuration;
	if (bUseEnemyData) Enemy->GetFirePattern(Pattern, Burst, Rest);

	FBTFireAtTargetMemory* Mem = CastInstanceNodeMemory<FBTFireAtTargetMemory>(NodeMemory);
	Mem->RemainingTime = FMath::Max(0.1f, Burst + FMath::FRandRange(-DurationDeviation, DurationDeviation));
	Mem->RestTime      = FMath::Max(0.f,  Rest  + FMath::FRandRange(-DurationDeviation, DurationDeviation));
	Mem->bResting      = false;

	// 새로 교전을 시작하는 경우에만 겨누는 시간을 준다 (연사 도중에는 끊지 않는다).
	// 진행 상황은 폰이 들고 있어서, 태스크가 중간에 끊겼다 다시 들어와도 이어서 센다.
	AActor* AimTargetActor = bVectorKey ? nullptr : Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	const bool bFreshEngage = Enemy->GetWorld()->TimeSince(Enemy->GetLastFireTime()) > ReengageGap;
	const bool bAimReady    = !bFreshEngage || Enemy->UpdateAimReady(AimTargetActor, AimDelay);
	Mem->AimTime = bAimReady ? 0.f : AimDelay;
	Mem->bFiring = false;

	// 겨누는 동안 타겟을 바라본다 (포커스 서비스가 없더라도 최소한 이 태스크에서는 조준한다)
	if (bVectorKey) AICon->SetFocalPoint(AimLocation);
	else if (AActor* FocusTarget = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName))) AICon->SetFocus(FocusTarget);

	if (Mem->AimTime <= 0.f)
	{
		Mem->bFiring = true;
		Enemy->FireAtTarget();
		Enemy->OnAttack();   // BP 훅: 사격 몽타주·머즐 이펙트 등
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_FireAtTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	AEnemyCharacter* Enemy = AICon ? Cast<AEnemyCharacter>(AICon->GetPawn()) : nullptr;
	AWeaponBase* Weapon = Enemy ? Enemy->GetEnemyWeapon() : nullptr;
	if (!Weapon)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 사격 도중에 넉백으로 쓰러졌으면 즉시 끊는다.
	// ExecuteTask만 막으면 이미 진행 중이던 태스크는 그대로 계속 쏜다
	if (Enemy->IsIncapacitated())
	{
		Finish(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FBTFireAtTargetMemory* Mem = CastInstanceNodeMemory<FBTFireAtTargetMemory>(NodeMemory);

	// 겨누는 중 — 다 겨누면 그때 쏘기 시작한다 (판정은 폰이 기억하는 진행 상황 기준)
	if (Mem->AimTime > 0.f && !Mem->bFiring)
	{
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		AActor* AimTargetActor = BB ? Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;
		if (Enemy->UpdateAimReady(AimTargetActor, AimDelay))
		{
			Mem->AimTime = 0.f;
			Mem->bFiring = true;
			Enemy->FireAtTarget();
			Enemy->OnAttack();
		}
		return;
	}

	if (Mem->bResting)
	{
		Mem->RestTime -= DeltaSeconds;
		if (Mem->RestTime <= 0.f) FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 사격 도중 탄창이 비면 휴식 없이 즉시 재장전 분기로
	if (!Weapon->CanFire())
	{
		if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
		{
			if (!Weapon->IsReloading()) BB->SetValueAsBool(AEnemyCharacter::BBKey_bNeedsReload, true);
		}
		Finish(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	Mem->RemainingTime -= DeltaSeconds;
	if (Mem->RemainingTime <= 0.f)
	{
		Enemy->StopFiring();
		if (Mem->RestTime <= 0.f)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
		Mem->bResting = true;
		return;
	}

	// 연사 무기는 무기 타이머가 쏘고 있다. 단발·점사 무기만 준비될 때마다 다시 당긴다
	if (!Weapon->IsAutoFire() && Weapon->IsFireReady())
	{
		Enemy->FireAtTarget();
		Enemy->OnAttack();
	}
}

EBTNodeResult::Type UBTTask_FireAtTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 타겟을 놓쳐 상위에서 중단시킨 경우에도 총구는 반드시 멈춘다
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(AICon->GetPawn()))
		{
			Enemy->StopFiring();
		}
	}
	return EBTNodeResult::Aborted;
}

void UBTTask_FireAtTarget::Finish(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type Result) const
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(AICon->GetPawn()))
		{
			Enemy->StopFiring();
		}
	}
	FinishLatentTask(OwnerComp, Result);
}
