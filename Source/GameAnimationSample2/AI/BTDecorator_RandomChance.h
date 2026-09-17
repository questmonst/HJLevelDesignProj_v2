// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "EnemyCharacter.h"
#include "BTDecorator_RandomChance.generated.h"

// 진입할 때마다 확률로 분기를 허용하는 데코레이터.
//
// 엔진 기본 데코레이터에는 확률 분기가 없다(UE 5.7 확인). "30% 확률로 스트레이핑 사격"처럼
// 행동 패턴에 변주를 주는 용도. 결과는 진입 시점에 한 번만 굴린다 — 매 틱 다시 굴리면
// 실행 중인 분기가 들쭉날쭉 끊기므로 Observer aborts는 허용하지 않는다.
UCLASS()
class GAMEANIMATIONSAMPLE2_API UBTDecorator_RandomChance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_RandomChance();

	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "Condition", meta=(ToolTip="true면 적의 EnemyData 확률을 쓴다 — 같은 BT를 AR·MG 등이 공유해도 적마다 성향이 달라지게"))
	bool bUseEnemyData = true;

	UPROPERTY(EditAnywhere, Category = "Condition", meta=(EditCondition="bUseEnemyData", ToolTip="EnemyData에서 읽을 확률 항목"))
	EEnemyBehaviorChance Behavior = EEnemyBehaviorChance::Strafe;

	UPROPERTY(EditAnywhere, Category = "Condition", meta=(EditCondition="!bUseEnemyData", ClampMin="0", ClampMax="100", ToolTip="이 분기에 진입할 확률 (%). EnemyData를 안 쓸 때"))
	float ChancePercent = 30.f;
};
