// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProjectileBase.h"
#include "WeaponDataAsset.generated.h"

class UNiagaraSystem;

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Hitscan    UMETA(DisplayName = "Hitscan"),
	Projectile UMETA(DisplayName = "Projectile"),
};

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Config ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	EWeaponFireMode FireMode = EWeaponFireMode::Hitscan;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bIsAutoFire = false;

	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float FireRate = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float Range = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float ReloadTime = 2.f;

	// --- Ammo ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 MagSize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 ReserveAmmo = 90;

	// --- Projectile ---

	/** Hitscan 모드에서 장식용으로 스폰할 투사체 (충돌·대미지 없음) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AProjectileBase> CosmeticProjectileClass;

	/** 0이면 투사체 기본값 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float ProjectileSpeedOverride = 0.f;

	// --- Spread ---

	/** 발사 1회당 추가되는 크로스헤어 스프레드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spread")
	float SpreadPerShot = 2.f;

	/** 장전 시작 시 추가되는 크로스헤어 스프레드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spread")
	float SpreadReloading = 5.f;

	/** 초당 스프레드 회복량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spread")
	float SpreadRecoverySpeed = 15.f;

	// --- Config ---

	/** 플레이어 발사 시 카메라 트레이스 시작점 오프셋 (cm). 엄폐물 클리핑 방지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	float TraceStartOffset = 30.f;

	/** 발사 트레이스 디버그 드로우 활성화 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bDebugTrace = false;

	// --- Audio ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* FireSound = nullptr;

	// --- Hit VFX ---

	/** 히트스캔 탄착점에 스폰할 나이아가라 시스템 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* HitVFX = nullptr;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* MuzzleVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	float MuzzleVFXScale = 1.f;
};
