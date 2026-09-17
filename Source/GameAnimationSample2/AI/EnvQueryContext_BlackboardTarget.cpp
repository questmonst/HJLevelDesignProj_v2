// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnvQueryContext_BlackboardTarget.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

void UEnvQueryContext_BlackboardTarget::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	const AAIController* AICon = QuerierPawn ? Cast<AAIController>(QuerierPawn->GetController()) : nullptr;
	const UBlackboardComponent* BB = AICon ? AICon->GetBlackboardComponent() : nullptr;
	if (!BB) return;

	if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKeyName)))
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
	}
}
