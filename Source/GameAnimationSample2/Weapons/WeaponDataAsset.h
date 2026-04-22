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

	// --- Audio ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* FireSound = nullptr;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* MuzzleVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	float MuzzleVFXScale = 1.f;
};
