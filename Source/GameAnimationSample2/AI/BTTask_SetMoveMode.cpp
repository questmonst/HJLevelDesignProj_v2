// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_SetMoveMode.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"

UBTTask_SetMoveMode::UBTTask_SetMoveMode()
{
	NodeName = TEXT("Set Move Mode");
	bCreateNodeInstance = false;
}

FString UBTTask_SetMoveMode::GetStaticDescription() const
{
	return FString::Printf(TEXT("이동 자세: %s"), *UEnum::GetDisplayValueAsText(MoveMode).ToString());
}

EBTNodeResult::Type UBTTask_SetMoveMode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	AEnemyCharacter* Enemy = AICon ? Cast<AEnemyCharacter>(AICon->GetPawn()) : nullptr;
	if (!Enemy) return EBTNodeResult::Failed;

	Enemy->SetMoveMode(MoveMode);
	return EBTNodeResult::Succeeded;
}
