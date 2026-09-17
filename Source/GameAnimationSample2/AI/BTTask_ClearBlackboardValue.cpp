// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_ClearBlackboardValue.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearBlackboardValue::UBTTask_ClearBlackboardValue()
{
	NodeName = TEXT("Clear Blackboard Value");
	bCreateNodeInstance = false;
}

FString UBTTask_ClearBlackboardValue::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s 비우기"), *KeyToClear.SelectedKeyName.ToString());
}

EBTNodeResult::Type UBTTask_ClearBlackboardValue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB || KeyToClear.SelectedKeyName.IsNone()) return EBTNodeResult::Failed;

	BB->ClearValue(KeyToClear.SelectedKeyName);
	return EBTNodeResult::Succeeded;
}
