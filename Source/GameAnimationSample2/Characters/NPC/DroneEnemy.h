// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseAirUnit.h"
#include "DroneEnemy.generated.h"

/** 드론 — 지속 압박형: 실내/열차 내부, 엄폐 무효화 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ADroneEnemy : public ABaseAirUnit
{
    GENERATED_BODY()
public:
    ADroneEnemy();
};
