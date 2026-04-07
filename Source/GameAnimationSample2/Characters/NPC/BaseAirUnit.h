// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "BaseAirUnit.generated.h"

/**
 * 공중 유닛 베이스.
 * CharacterMovement를 Flying 모드로 설정.
 * 엄폐 행동 없음, 호버 높이 유지.
 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ABaseAirUnit : public AEnemyCharacter
{
    GENERATED_BODY()

public:
    ABaseAirUnit();

protected:
    virtual void BeginPlay() override;

    /** 지면으로부터 유지할 호버 높이 (cm). BT에서 이동 목표 Z 보정에 사용 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AirUnit")
    float HoverHeight = 400.f;

    /** 공중 이동 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AirUnit")
    float FlySpeed = 600.f;
};
