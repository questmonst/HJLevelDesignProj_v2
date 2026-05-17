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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret", meta=(ToolTip="포탑 루트 컴포넌트"))
    USceneComponent* TurretRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret", meta=(ToolTip="회전하는 포신 피벗"))
    USceneComponent* BarrelPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret", meta=(ToolTip="AI 감지 컴포넌트"))
    UAIPerceptionComponent* AIPerception;

    // --- Stats ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats", meta=(ToolTip="포탑 최대 체력"))
    float MaxHealth = 80.f;

    UPROPERTY(BlueprintReadOnly, Category = "Turret|Stats", meta=(ToolTip="현재 체력"))
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats", meta=(ToolTip="발사 1회당 피해량"))
    float AttackDamage = 12.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats", meta=(ToolTip="공격 가능 최대 사거리 (cm)"))
    float AttackRange = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats", meta=(ToolTip="공격 쿨다운 (초)"))
    float AttackCooldown = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats", meta=(ToolTip="포신 회전 속도 (도/초)"))
    float RotationSpeed = 90.f;

    // --- Weapon ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Weapon", meta=(ToolTip="포탑에 장착할 무기 클래스"))
    TSubclassOf<AWeaponBase> WeaponClass;

    UPROPERTY(BlueprintReadOnly, Category = "Turret|Weapon", meta=(ToolTip="현재 장착된 무기 레퍼런스"))
    AWeaponBase* TurretWeapon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Weapon", meta=(ToolTip="무기를 부착할 소켓 이름"))
    FName WeaponAttachSocket = TEXT("Muzzle");

    // --- State ---

    UPROPERTY(BlueprintReadOnly, Category = "Turret", meta=(ToolTip="현재 공격 대상"))
    AActor* CurrentTarget = nullptr;

    bool bIsFiring = false;
    float FireCooldownRemaining = 0.f;

public:
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

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
