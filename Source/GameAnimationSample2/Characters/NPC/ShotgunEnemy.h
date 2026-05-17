// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "ShotgunEnemy.generated.h"

/** 샷건 — 기회주의형: 원거리 전투하다 일정 시간 후 돌격 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AShotgunEnemy : public AEnemyCharacter
{
    GENERATED_BODY()
public:
    AShotgunEnemy();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun|AI", meta=(ToolTip="이 거리 이하로 접근하면 근접 돌격 전환 (cm). BT에서 읽음"))
    float ChargeRange = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun|AI", meta=(ToolTip="교전 시작 후 이 시간(초) 이상 지나면 돌격. BT에서 읽음"))
    float ChargeAfterEngageTime = 5.f;
};
