// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "EnemyCharacter.h"
#include "BTTask_SetMoveMode.generated.h"

// 적의 이동 자세(걷기/달리기/앉기)를 바꾸는 즉시 완료 태스크.
//
// 속도 수치는 태스크가 아니라 캐릭터(EnemyData)가 들고 있다 — 같은 BT를 적 종류끼리 공유해도
// "달리기"의 실제 속도는 적마다 다르게 하기 위해서다.
// Move To 앞에 두어 "엄폐물까지는 달리고, 도착하면 앉는" 흐름을 만든다.
UCLASS()
class GAMEANIMATIONSAMPLE2_API UBTTask_SetMoveMode : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetMoveMode();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Movement", meta=(ToolTip="바꿀 이동 자세. 속도는 EnemyData의 Walk/Run/CrouchWalkSpeed"))
	EEnemyMoveMode MoveMode = EEnemyMoveMode::Run;
};
