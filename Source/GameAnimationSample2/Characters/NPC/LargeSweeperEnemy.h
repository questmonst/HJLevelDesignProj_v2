// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "LargeSweeperEnemy.generated.h"

/** 대형 스위퍼 — 공간 지배형: 넓은 근접 범위, 미카 랜딩 유도 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ALargeSweeperEnemy : public AEnemyCharacter
{
    GENERATED_BODY()
public:
    ALargeSweeperEnemy();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweeper|Combat", meta=(ToolTip="근접 스윕 반경 (cm). BT에서 이 범위 내 플레이어에게 근접 공격"))
    float MeleeSwipeRadius = 300.f;
};
