// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_FireAtTarget.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AIController.h"
#include "EnemyCharacter.h"

UBTTask_FireAtTarget::UBTTask_FireAtTarget()
{
	NodeName     = TEXT("Fire At Target");
	bNotifyTick  = true;
	bCreateNodeInstance = false;

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FireAtTarget, TargetKey), AActor::StaticClass());
}

uint16 UBTTask_FireAtTarget::GetInstanceMemorySize() const
{
	return sizeof(FBTFireAtTargetMemory);
}

FString UBTTask_FireAtTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s을(를) %s 동안 사격"),
		*TargetKey.SelectedKeyName.ToString(),
		FireDuration > 0.f ? *FString::Printf(TEXT("%.2f초"), FireDuration) : TEXT("AttackCooldown"));
}

EBTNodeResult::Type UBTTask_FireAtTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	AEnemyCharacter* Enemy = AICon ? Cast<AEnemyCharacter>(AICon->GetPawn()) : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Enemy || !BB) return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!Target) return EBTNodeResult::Failed;

	// 사거리 밖이면 쏘지 않는다 — 실패시키면 상위 Sequence가 끊겨 다시 접근 단계로 돌아간다
	const float Dist = FVector::Dist(Enemy->GetActorLocation(), Target->GetActorLocation());
	if (Dist > Enemy->GetAttackRange())
	{
		return bFailWhenOutOfRange ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;
	}

	// 벽 너머로 쏘지 않도록 — AIController의 시야 판정을 그대로 쓴다
	if (bRequireLineOfSight && !AICon->LineOfSightTo(Target))
	{
		return bFailWhenOutOfRange ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;
	}

	Enemy->FireAtTarget();
	Enemy->OnAttack();   // BP 훅: 사격 몽타주·머즐 이펙트 등

	FBTFireAtTargetMemory* Mem = reinterpret_cast<FBTFireAtTargetMemory*>(NodeMemory);
	Mem->RemainingTime = FireDuration > 0.f ? FireDuration : Enemy->GetAttackCooldown();

	return EBTNodeResult::InProgress;
}

void UBTTask_FireAtTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTFireAtTargetMemory* Mem = reinterpret_cast<FBTFireAtTargetMemory*>(NodeMemory);
	Mem->RemainingTime -= DeltaSeconds;

	if (Mem->RemainingTime > 0.f) return;

	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(AICon->GetPawn()))
		{
			Enemy->StopFiring();
		}
	}
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
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
