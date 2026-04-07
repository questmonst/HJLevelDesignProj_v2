// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "MGEnemy.generated.h"

/** MG — 개활지 방어형: 강한 화력으로 전진 차단, 위치 고수 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AMGEnemy : public AEnemyCharacter
{
    GENERATED_BODY()
public:
    AMGEnemy();

    /** true면 BT가 PatrolOrigin 근처에서 이동하지 않음 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MG|AI")
    bool bHoldPosition = true;
};
