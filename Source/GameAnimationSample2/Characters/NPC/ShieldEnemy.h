// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "ShieldEnemy.generated.h"

/**
 * 쉴드 — 전선 형성형.
 * 쉴드 체력이 존재하며, 쉴드가 살아있는 동안 전면 데미지를 쉴드가 흡수.
 * 미카 펀치 또는 미유 공격이 없으면 처리 불가.
 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AShieldEnemy : public AEnemyCharacter
{
    GENERATED_BODY()

public:
    AShieldEnemy();

protected:
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield")
    float ShieldMaxHealth = 150.f;

    UPROPERTY(BlueprintReadOnly, Category = "Shield")
    float ShieldCurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Shield")
    bool bShieldActive = true;

    /** 전면 쉴드 차단 각도 (도, 이 각도 이내의 전방 공격을 쉴드가 흡수) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield")
    float ShieldBlockAngle = 60.f;

public:
    UFUNCTION(BlueprintPure, Category = "Shield")
    bool IsShieldActive() const { return bShieldActive; }

    UFUNCTION(BlueprintPure, Category = "Shield")
    float GetShieldPercent() const;

    /** 쉴드가 파괴됐을 때 BP에서 오버라이드 (이펙트, 사운드 등) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shield")
    void OnShieldBreak();
    virtual void OnShieldBreak_Implementation();
};
