// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "SniperEnemy.generated.h"

/** 저격 — 고지대 위협형: 고지대 고정, 장거리 사격 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ASniperEnemy : public AEnemyCharacter
{
    GENERATED_BODY()
public:
    ASniperEnemy();

    /** true면 BT가 PatrolOrigin 근처에 고정 (이동 안 함) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sniper|AI")
    bool bHoldPosition = true;
};
