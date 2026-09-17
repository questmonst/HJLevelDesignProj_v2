// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ClearBlackboardValue.generated.h"

// 블랙보드 키 하나를 비우는 즉시 완료 태스크.
//
// 엔진에 기본 제공 노드가 없다. "마지막 목격 위치 조사 후 지우기", "엄폐 끝나면 CoverLocation 지우기"처럼
// Is Set / Is Not Set 데코레이터로 분기를 여닫는 패턴에 필요하다.
UCLASS()
class GAMEANIMATIONSAMPLE2_API UBTTask_ClearBlackboardValue : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ClearBlackboardValue();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta=(ToolTip="비울 블랙보드 키 (타입 무관)"))
	FBlackboardKeySelector KeyToClear;
};
