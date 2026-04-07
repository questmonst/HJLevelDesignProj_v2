// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "AREnemy.generated.h"

/** AR — 압박형: 중거리 유지, 엄폐 → 사격 반복 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AAREnemy : public AEnemyCharacter
{
    GENERATED_BODY()
public:
    AAREnemy();
};
