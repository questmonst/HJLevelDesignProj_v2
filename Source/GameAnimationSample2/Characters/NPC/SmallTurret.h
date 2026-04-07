// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseTurret.h"
#include "SmallTurret.generated.h"

/** 소형 포탑 — 통로 차단형: 미카 펀치로 즉각 파괴 가능 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ASmallTurret : public ABaseTurret
{
    GENERATED_BODY()
public:
    ASmallTurret();
};
