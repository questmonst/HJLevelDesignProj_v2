// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_FindStrafeLocation.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "AIController.h"
#include "NavigationSystem.h"

UBTTask_FindStrafeLocation::UBTTask_FindStrafeLocation()
{
	NodeName = TEXT("Find Strafe Location");
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindStrafeLocation, TargetKey), AActor::StaticClass());
	ResultKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindStrafeLocation, ResultKey));
}

void UBTTask_FindStrafeLocation::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (const UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
		ResultKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTTask_FindStrafeLocation::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s 기준 옆 %.0f~%.0fcm → %s"),
		*TargetKey.SelectedKeyName.ToString(), MinDistance, MaxDistance, *ResultKey.SelectedKeyName.ToString());
}

EBTNodeResult::Type UBTTask_FindStrafeLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Pawn || !BB) return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());
	if (!Target || !NavSys) return EBTNodeResult::Failed;

	const FVector Origin   = Pawn->GetActorLocation();
	const FVector ToTarget = (Target->GetActorLocation() - Origin).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) return EBTNodeResult::Failed;
	const FVector Side = FVector::CrossProduct(FVector::UpVector, ToTarget);   // 타겟 기준 오른쪽

	FCollisionQueryParams Params(SCENE_QUERY_STAT(StrafeLOS), false, Pawn);
	Params.AddIgnoredActor(Target);

	float Sign = FMath::RandBool() ? 1.f : -1.f;
	for (int32 i = 0; i < MaxTries; ++i)
	{
		const float Dist = FMath::FRandRange(MinDistance, FMath::Max(MinDistance, MaxDistance));
		FNavLocation NavLoc;
		if (NavSys->ProjectPointToNavigation(Origin + Side * Sign * Dist, NavLoc))
		{
			// 도착해서도 타겟이 보이는 자리만 (벽 뒤로 숨어 버리면 사격이 끊긴다)
			const FVector Eye = NavLoc.Location + FVector(0.f, 0.f, Pawn->BaseEyeHeight);
			if (!bRequireLineOfSight ||
				!Pawn->GetWorld()->LineTraceTestByChannel(Eye, Target->GetActorLocation(), ECC_Visibility, Params))
			{
				BB->SetValueAsVector(ResultKey.SelectedKeyName, NavLoc.Location);
				return EBTNodeResult::Succeeded;
			}
		}
		Sign = -Sign;   // 반대쪽도 번갈아 시도
	}
	return EBTNodeResult::Failed;
}
