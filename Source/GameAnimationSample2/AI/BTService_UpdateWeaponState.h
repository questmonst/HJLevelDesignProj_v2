// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateWeaponState.generated.h"

// 무기 탄약 상태를 블랙보드 Bool(bNeedsReload)로 옮겨 적는 서비스.
//
// BT 데코레이터는 무기 액터 내부를 직접 볼 수 없다. 블랙보드에 올려두면 기본 제공
// Blackboard Based Condition(+ Observer aborts)만으로 "탄이 떨어지면 사격을 끊고 엄폐 재장전"
// 분기를 만들 수 있다. 루트 Selector에 붙여 트리 전체에서 항상 갱신되게 한다.
UCLASS()
class GAMEANIMATIONSAMPLE2_API UBTService_UpdateWeaponState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateWeaponState();

	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Weapon", meta=(ToolTip="재장전 필요 여부를 쓸 Bool 키. 보통 bNeedsReload"))
	FBlackboardKeySelector NeedsReloadKey;

private:
	void UpdateKeys(UBehaviorTreeComponent& OwnerComp) const;
};
