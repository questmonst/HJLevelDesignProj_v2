//// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.h"
#include "WeaponDataAsset.h"
#include "WeaponBase.generated.h"

class UNiagaraSystem;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	// --- Data ---

	/** 할당 시 BeginPlay에서 Stats·Audio·VFX 값을 덮어씀 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data")
	UWeaponDataAsset* WeaponData = nullptr;

	// --- Config ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config")
	EWeaponFireMode FireMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config")
	bool bIsAutoFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config")
	FName MuzzleSocketName;

	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float FireRate; // rounds per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float Range; // hitscan max range (cm)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float ReloadTime;

	// --- Ammo ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MagSize;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 ReserveAmmo;

	// --- Projectile ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	TSubclassOf<AProjectileBase> ProjectileClass;

	/** Hitscan 모드에서 장식용으로 스폰할 투사체 (충돌·대미지 없음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	TSubclassOf<AProjectileBase> CosmeticProjectileClass;

	/** 0이면 투사체 기본값 사용, 양수면 이 속도로 덮어씀 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	float ProjectileSpeedOverride = 0.f;

	// --- Spread ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Spread")
	float SpreadPerShot = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Spread")
	float SpreadReloading = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Spread")
	float SpreadRecoverySpeed = 15.f;

	/** 플레이어 발사 시 카메라에서 트레이스 시작점을 앞으로 밀어 엄폐물 클리핑 방지 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config")
	float TraceStartOffset = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config")
	bool bDebugTrace = false;

	// --- Audio ---

	/** 발사 시 재생할 사운드 (SoundWave / SoundCue 모두 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Audio")
	USoundBase* FireSound = nullptr;

	// --- Hit VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
	UNiagaraSystem* HitVFX = nullptr;

	// --- VFX ---

	/** 발사 시 머즐에 스폰할 나이아가라 시스템 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
	UNiagaraSystem* MuzzleVFX = nullptr;

	/** 머즐 VFX 스케일 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
	float MuzzleVFXScale = 1.0f;

	// --- Internal state ---

	bool bIsReloading;
	FTimerHandle AutoFireTimerHandle;
	FTimerHandle ReloadTimerHandle;

	void HitscanFire();
	void ProjectileFire();
	void FinishReload();

public:
	// --- Fire ---

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();

	// --- Reload ---

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	int32 GetReserveAmmo() const { return ReserveAmmo; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	bool IsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Spread")
	float GetSpreadRecoverySpeed() const { return SpreadRecoverySpeed; }

	// --- Events (override in Blueprint for VFX/SFX) ---

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	void OnFire(const FHitResult& HitResult);
	virtual void OnFire_Implementation(const FHitResult& HitResult);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	void OnReloadStart();
	virtual void OnReloadStart_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	void OnReloadFinish();
	virtual void OnReloadFinish_Implementation();
};
