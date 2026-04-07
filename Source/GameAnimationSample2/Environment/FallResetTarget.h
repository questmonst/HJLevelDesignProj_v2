// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallResetTarget.generated.h"

/**
 * 낙사 리셋 목표 지점 (Actor B).
 * FallResetTrigger가 이 액터의 위치/방향으로 플레이어를 이동시킴.
 * 에디터에서 Arrow로 방향(앞쪽)을 확인할 수 있음.
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API AFallResetTarget : public AActor
{
    GENERATED_BODY()

public:
    AFallResetTarget();

private:
    UPROPERTY(VisibleAnywhere)
    USceneComponent* SceneRoot;

#if WITH_EDITORONLY_DATA
    UPROPERTY()
    class UArrowComponent* DirectionArrow;
#endif
};
