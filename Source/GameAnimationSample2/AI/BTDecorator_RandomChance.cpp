// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTDecorator_RandomChance.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"

UBTDecorator_RandomChance::UBTDecorator_RandomChance()
{
	NodeName = TEXT("Random Chance");

	// 진입 시 한 번만 판정 — 재평가로 실행 중인 분기를 끊지 않는다
	bAllowAbortNone       = true;
	bAllowAbortLowerPri   = false;
	bAllowAbortChildNodes = false;
}

FString UBTDecorator_RandomChance::GetStaticDescription() const
{
	const FString Chance = bUseEnemyData
		? FString::Printf(TEXT("EnemyData %s"), *UEnum::GetDisplayValueAsText(Behavior).ToString())
		: FString::Printf(TEXT("%.0f%%"), ChancePercent);
	return FString::Printf(TEXT("%s%s 확률"), IsInversed() ? TEXT("NOT ") : TEXT(""), *Chance);
}

bool UBTDecorator_RandomChance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	float Chance = ChancePercent;
	if (bUseEnemyData)
	{
		const AAIController* AICon = OwnerComp.GetAIOwner();
		const AEnemyCharacter* Enemy = AICon ? Cast<AEnemyCharacter>(AICon->GetPawn()) : nullptr;
		if (!Enemy) return false;
		Chance = Enemy->GetBehaviorChance(Behavior);
	}
	return FMath::FRand() * 100.f < Chance;
}
