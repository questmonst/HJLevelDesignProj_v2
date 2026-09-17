// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTService_UpdateWeaponState.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "EnemyCharacter.h"

UBTService_UpdateWeaponState::UBTService_UpdateWeaponState()
{
	NodeName = TEXT("Update Weapon State");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
	bNotifyBecomeRelevant = true;
	bCreateNodeInstance = false;

	NeedsReloadKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateWeaponState, NeedsReloadKey));
	NeedsReloadKey.SelectedKeyName = AEnemyCharacter::BBKey_bNeedsReload;
}

FString UBTService_UpdateWeaponState::GetStaticDescription() const
{
	return FString::Printf(TEXT("잔탄 확인 → %s\n%s"),
		*NeedsReloadKey.SelectedKeyName.ToString(), *Super::GetStaticDescription());
}

void UBTService_UpdateWeaponState::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UpdateKeys(OwnerComp);
}

void UBTService_UpdateWeaponState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	UpdateKeys(OwnerComp);
}

void UBTService_UpdateWeaponState::UpdateKeys(UBehaviorTreeComponent& OwnerComp) const
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	const AEnemyCharacter* Enemy = AICon ? Cast<AEnemyCharacter>(AICon->GetPawn()) : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Enemy || !BB || NeedsReloadKey.SelectedKeyName.IsNone()) return;

	// 장전 중에는 true를 유지 — 여기서 내리면 재장전 분기가 중간에 끊겨 엄폐를 풀어버린다.
	// 완료 후 false로 내리는 건 Reload Weapon 태스크 몫.
	if (Enemy->IsReloading()) return;

	BB->SetValueAsBool(NeedsReloadKey.SelectedKeyName, Enemy->NeedsReload());
}
