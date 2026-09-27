// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindStrafeLocation.generated.h"

// 타겟을 바라본 채 옆으로 이동할 지점을 찾는다 (스트레이핑 사격용).
//
// 타겟 방향에 수직인 좌/우 중 무작위 한쪽으로 Min~Max 거리 떨어진 점을 내비메시에 투영하고,
// 거기서 타겟이 보이면 결과 키에 넣는다. 안 되면 반대쪽·다른 거리로 몇 번 더 시도한다.
// Simple Parallel의 배경 가지에서 Move To와 함께 쓴다 (메인은 Fire At Target).
UCLASS()
class GAMEANIMATIONSAMPLE2_API UBTTask_FindStrafeLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindStrafeLocation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Strafe", meta=(ToolTip="바라볼 타겟 (Object 키)"))
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, Category = "Strafe", meta=(ToolTip="찾은 지점을 넣을 Vector 키"))
	FBlackboardKeySelector ResultKey;

	UPROPERTY(EditAnywhere, Category = "Strafe", meta=(ClampMin="0", ToolTip="옆으로 이동할 최소 거리 (cm)"))
	float MinDistance = 200.f;

	UPROPERTY(EditAnywhere, Category = "Strafe", meta=(ClampMin="0", ToolTip="옆으로 이동할 최대 거리 (cm)"))
	float MaxDistance = 450.f;

	UPROPERTY(EditAnywhere, Category = "Strafe", meta=(ToolTip="true면 도착 지점에서 타겟이 보일 때만 고른다"))
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, Category = "Strafe", meta=(ClampMin="1", ToolTip="지점 찾기 시도 횟수"))
	int32 MaxTries = 6;
};
