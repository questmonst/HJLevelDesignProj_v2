// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IDestructible.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDestructibleObject : public UInterface
{
    GENERATED_BODY()
};

/**
 * 미카 펀치에 의해 즉각 파괴될 수 있는 오브젝트 인터페이스.
 * DestructibleCover 등 환경 파괴물에 구현.
 */
class GAMEANIMATIONSAMPLE2_API IDestructibleObject
{
    GENERATED_BODY()

public:
    /**
     * 미카 펀치 히트 시 호출.
     * 구현체에서 파괴 연출(Chaos, VFX 등)을 처리하고 액터를 제거.
     * @param Instigator  펀치를 시전한 캐릭터
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Destructible")
    void DestroyByPunch(AActor* PunchInstigator);
};
