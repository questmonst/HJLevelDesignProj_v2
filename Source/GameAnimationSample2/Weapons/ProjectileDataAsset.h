// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProjectileDataAsset.generated.h"

class UNiagaraSystem;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UProjectileDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float InitialSpeed = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float GravityScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float LifeSpanSeconds = 3.f;

	// --- Audio ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* HitSound = nullptr;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* FlightVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	float FlightVFXScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	float HitVFXScale = 1.f;
};
