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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="투사체 피해량"))
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="초기 발사 속도 (cm/s)"))
	float InitialSpeed = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="중력 배율 (0=무중력, 1=기본 중력)"))
	float GravityScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="투사체 수명 (초). 이 시간 후 자동 소멸"))
	float LifeSpanSeconds = 3.f;

	// --- Audio ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta=(ToolTip="충돌 시 재생할 사운드 에셋"))
	USoundBase* HitSound = nullptr;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="비행 중 투사체에 붙일 나이아가라 시스템 (트레일 등)"))
	UNiagaraSystem* FlightVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="비행 VFX 스케일"))
	float FlightVFXScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="충돌 시 히트 위치에 스폰할 나이아가라 시스템"))
	UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="히트 VFX 스케일"))
	float HitVFXScale = 1.f;
};
