// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseTurret.generated.h"

class AWeaponBase;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 * 고정 포탑 베이스.
 * 이동 없음, 타겟 방향으로 회전 후 사격.
 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ABaseTurret : public AActor
{
    GENERATED_BODY()

public:
    ABaseTurret();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret")
    USceneComponent* TurretRoot;

    /** 회전하는 포신 피벗 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret")
    USceneComponent* BarrelPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret")
    UAIPerceptionComponent* AIPerception;

    // --- Stats ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats")
    float MaxHealth = 80.f;

    UPROPERTY(BlueprintReadOnly, Category = "Turret|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats")
    float AttackDamage = 12.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats")
    float AttackRange = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats")
    float AttackCooldown = 0.5f;

    /** 포신 회전 속도 (도/초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats")
    float RotationSpeed = 90.f;

    // --- Weapon ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Weapon")
    TSubclassOf<AWeaponBase> WeaponClass;

    UPROPERTY(BlueprintReadOnly, Category = "Turret|Weapon")
    AWeaponBase* TurretWeapon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Weapon")
    FName WeaponAttachSocket = TEXT("Muzzle");

    // --- State ---

    UPROPERTY(BlueprintReadOnly, Category = "Turret")
    AActor* CurrentTarget = nullptr;

    bool bIsFiring = false;
    float FireCooldownRemaining = 0.f;

public:
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    /** 포탑 파괴 시 BP에서 오버라이드 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Turret")
    void OnTurretDestroyed();
    virtual void OnTurretDestroyed_Implementation();

private:
    UAISenseConfig_Sight* SightConfig = nullptr;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    void RotateTowardTarget(float DeltaTime);
    void TryFire(float DeltaTime);
};
