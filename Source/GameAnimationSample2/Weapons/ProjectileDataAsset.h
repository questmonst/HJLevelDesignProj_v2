// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProjectileDataAsset.generated.h"

class UNiagaraSystem;
class USoundAttenuation;
class USoundConcurrency;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UProjectileDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="투사체 피해량. 무기로 쏘면 무기 DA의 Damage로 덮어써진다 — 무기 없이 스폰될 때만 이 값"))
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="초기 발사 속도 (cm/s)"))
	float InitialSpeed = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="중력 배율 (0=무중력, 1=기본 중력)"))
	float GravityScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="투사체 수명 (초). 이 시간 후 자동 소멸"))
	float LifeSpanSeconds = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="true면 충돌 시 점 피해 대신 범위 폭발 피해를 적용 (수류탄 등)"))
	bool bRadialDamage = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="범위 폭발 반경 (cm). bRadialDamage=true일 때만 사용", EditCondition="bRadialDamage"))
	float DamageRadius = 300.f;

	// --- Audio ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta=(ToolTip="충돌 시 재생할 사운드 에셋"))
	USoundBase* HitSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta=(ToolTip="탄착음 거리 감쇠. 비우면 사운드 에셋 자체 설정 (SA_SFX = 5m까지 그대로, 30m에서 거의 안 들림)"))
	USoundAttenuation* HitSoundAttenuation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta=(ToolTip="탄착음 동시 재생 제한. 비우면 사운드 에셋 자체 설정 (SC_Impact = 전체 4개, 오래된 것부터 끊음)"))
	USoundConcurrency* HitSoundConcurrency = nullptr;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="비행 중 투사체에 붙일 나이아가라 시스템 (트레일 등)"))
	UNiagaraSystem* FlightVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="비행 VFX 스케일"))
	float FlightVFXScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="충돌 시 히트 위치에 스폰할 나이아가라 시스템"))
	UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="히트 VFX 스케일"))
	float HitVFXScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="캐릭터(적·플레이어)에 맞았을 때 스폰할 나이아가라 시스템. 캐릭터에는 HitVFX 대신 이걸 쓴다. 비우면 캐릭터 피격 이펙트 없음"))
	UNiagaraSystem* CharacterHitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="캐릭터 피격 VFX 스케일"))
	float CharacterHitVFXScale = 1.f;
};
