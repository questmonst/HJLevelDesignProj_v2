// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FireAtTarget.generated.h"

// 적이 블랙보드 타겟을 향해 일정 시간 사격하는 BT 태스크.
//
// 사격 자체는 AEnemyCharacter::FireAtTarget()이 이미 하고 있으므로, 이 태스크는
// "언제 시작해서 언제 멈추는가"만 책임진다. BP 태스크 대신 C++로 둔 이유는
// BT 노드 목록에 바로 떠서 BP 배선(Cast 노드 등)이 필요 없기 때문.
//
// 조준은 하지 않는다 — 무기가 폰의 시점 방향으로 쏘므로, 타겟을 바라보게 하는 것은
// 앞선 Rotate to face BB entry / MoveTo의 몫이다.
UCLASS()
class GAMEANIMATIONSAMPLE2_API UBTTask_FireAtTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FireAtTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Fire", meta=(ToolTip="사격 대상이 들어 있는 블랙보드 키 (Object). 보통 TargetActor"))
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, Category = "Fire", meta=(ToolTip="사격을 유지할 시간(초). 0이면 캐릭터의 AttackCooldown을 쓴다"))
	float FireDuration = 0.f;

	UPROPERTY(EditAnywhere, Category = "Fire", meta=(ToolTip="사거리(AttackRange) 밖이면 실패시킨다. false면 쏘지 않고 성공 처리"))
	bool bFailWhenOutOfRange = true;

	UPROPERTY(EditAnywhere, Category = "Fire", meta=(ToolTip="타겟이 시야에 없어도 쏜다. false면 벽 너머로는 쏘지 않는다"))
	bool bRequireLineOfSight = true;
};

// 사격 잔여 시간만 들고 있으면 되므로 노드 인스턴스 대신 메모리 구조체 사용
struct FBTFireAtTargetMemory
{
	float RemainingTime = 0.f;
};
