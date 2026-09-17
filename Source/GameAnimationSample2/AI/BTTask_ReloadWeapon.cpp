// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_ReloadWeapon.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "EnemyCharacter.h"

UBTTask_ReloadWeapon::UBTTask_ReloadWeapon()
{
	NodeName    = TEXT("Reload Weapon");
	bNotifyTick = true;
	bCreateNodeInstance = false;

	NeedsReloadKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ReloadWeapon, NeedsReloadKey));
	NeedsReloadKey.SelectedKeyName = AEnemyCharacter::BBKey_bNeedsReload;
}

FString UBTTask_ReloadWeapon::GetStaticDescription() const
{
	return FString::Printf(TEXT("재장전 완료까지 대기 → %s = false"), *NeedsReloadKey.SelectedKeyName.ToString());
}

EBTNodeResult::Type UBTTask_ReloadWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	AEnemyCharacter* Enemy = AICon ? Cast<AEnemyCharacter>(AICon->GetPawn()) : nullptr;
	if (!Enemy || !Enemy->GetEnemyWeapon()) return EBTNodeResult::Failed;

	// 이미 장전 중이면(사격 태스크 도중 시작된 경우 등) 끝나기만 기다린다
	if (Enemy->IsReloading() || Enemy->ReloadWeapon())
	{
		return EBTNodeResult::InProgress;
	}

	// 가득 찼거나 예비탄이 없어 장전할 게 없음 — 키만 정리하고 통과
	ClearNeedsReload(OwnerComp);
	return EBTNodeResult::Succeeded;
}

void UBTTask_ReloadWeapon::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	AEnemyCharacter* Enemy = AICon ? Cast<AEnemyCharacter>(AICon->GetPawn()) : nullptr;
	if (Enemy && Enemy->IsReloading()) return;

	ClearNeedsReload(OwnerComp);
	FinishLatentTask(OwnerComp, Enemy ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
}

void UBTTask_ReloadWeapon::ClearNeedsReload(UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB && !NeedsReloadKey.SelectedKeyName.IsNone())
	{
		BB->SetValueAsBool(NeedsReloadKey.SelectedKeyName, false);
	}
}
