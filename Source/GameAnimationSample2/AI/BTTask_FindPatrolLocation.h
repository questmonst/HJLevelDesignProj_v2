// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindPatrolLocation.generated.h"

// 순찰 원점 주변에서 NavMesh로 도달 가능한 랜덤 지점을 골라 블랙보드에 쓰는 태스크.
//
// 지금 BT는 PatrolOrigin(스폰 위치)으로만 돌아가서 순찰이 "제자리 대기"가 된다.
// 반경은 캐릭터의 PatrolRadius(EnemyData)를 기본으로 써서 적 종류별로 순찰 범위를 다르게 한다.
UCLASS()
class GAMEANIMATIONSAMPLE2_API UBTTask_FindPatrolLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindPatrolLocation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Patrol", meta=(ToolTip="순찰 원점 (Vector). 보통 PatrolOrigin"))
	FBlackboardKeySelector OriginKey;

	UPROPERTY(EditAnywhere, Category = "Patrol", meta=(ToolTip="찾은 지점을 쓸 키 (Vector). 보통 PatrolLocation"))
	FBlackboardKeySelector ResultKey;

	UPROPERTY(EditAnywhere, Category = "Patrol", meta=(ClampMin="0", ToolTip="검색 반경 (cm). 0이면 캐릭터의 PatrolRadius 사용"))
	float RadiusOverride = 0.f;
};
