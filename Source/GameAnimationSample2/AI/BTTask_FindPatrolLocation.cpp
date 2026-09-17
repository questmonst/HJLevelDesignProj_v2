// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_FindPatrolLocation.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "EnemyCharacter.h"

UBTTask_FindPatrolLocation::UBTTask_FindPatrolLocation()
{
	NodeName = TEXT("Find Patrol Location");
	bCreateNodeInstance = false;

	OriginKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindPatrolLocation, OriginKey));
	ResultKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindPatrolLocation, ResultKey));
	OriginKey.SelectedKeyName = AEnemyCharacter::BBKey_PatrolOrigin;
	ResultKey.SelectedKeyName = TEXT("PatrolLocation");
}

FString UBTTask_FindPatrolLocation::GetStaticDescription() const
{
	const FString Radius = RadiusOverride > 0.f ? FString::Printf(TEXT("%.0f"), RadiusOverride) : TEXT("PatrolRadius");
	return FString::Printf(TEXT("%s 주변 %s 내 랜덤 → %s"),
		*OriginKey.SelectedKeyName.ToString(), *Radius, *ResultKey.SelectedKeyName.ToString());
}

EBTNodeResult::Type UBTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Pawn || !BB || ResultKey.SelectedKeyName.IsNone()) return EBTNodeResult::Failed;

	FVector Origin = BB->GetValueAsVector(OriginKey.SelectedKeyName);
	if (!FAISystem::IsValidLocation(Origin)) Origin = Pawn->GetActorLocation();

	float Radius = RadiusOverride;
	if (Radius <= 0.f)
	{
		const AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Pawn);
		Radius = Enemy ? Enemy->GetPatrolRadius() : 500.f;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());
	FNavLocation Result;
	if (!NavSys || !NavSys->GetRandomReachablePointInRadius(Origin, Radius, Result))
	{
		// NavMesh 밖이면 조용히 실패 — 레벨에 NavMeshBoundsVolume이 덮였는지 확인할 것
		return EBTNodeResult::Failed;
	}

	BB->SetValueAsVector(ResultKey.SelectedKeyName, Result.Location);
	return EBTNodeResult::Succeeded;
}
