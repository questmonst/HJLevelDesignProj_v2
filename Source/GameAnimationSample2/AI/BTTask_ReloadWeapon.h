// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReloadWeapon.generated.h"

// 적 무기를 재장전하고 끝날 때까지 기다리는 BT 태스크.
//
// "엄폐물로 이동 → 앉기 → 재장전" 흐름에서 재장전이 끝나기 전에 다음 노드로 넘어가면
// 엄폐를 풀고 빈 총으로 뛰쳐나가게 된다. 그래서 무기의 장전 상태를 틱마다 확인해 완료를 기다린다.
// 완료 시 bNeedsReload 키를 내려서 재장전 분기의 데코레이터가 바로 풀리게 한다.
UCLASS()
class GAMEANIMATIONSAMPLE2_API UBTTask_ReloadWeapon : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ReloadWeapon();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Reload", meta=(ToolTip="완료 시 false로 내릴 Bool 키. 보통 bNeedsReload. 비우면 건드리지 않음"))
	FBlackboardKeySelector NeedsReloadKey;

private:
	void ClearNeedsReload(UBehaviorTreeComponent& OwnerComp) const;
};
