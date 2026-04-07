// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseAirUnit.h"
#include "HelicopterEnemy.generated.h"

/** 헬기 — 광역 위협형: 야외 구간, 미유 사격으로 격추 가능 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AHelicopterEnemy : public ABaseAirUnit
{
    GENERATED_BODY()
public:
    AHelicopterEnemy();
};
