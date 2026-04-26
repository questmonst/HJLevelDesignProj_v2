// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

class UNiagaraSystem;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
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

	// --- Audio ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* FireSound = nullptr;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* MuzzleVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	float MuzzleVFXScale = 1.f;
};
