// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GrenadeDataAsset.generated.h"

class UNiagaraSystem;
class USoundBase;

// 폭발 방식: 투척 수류탄은 신관(시간), 유탄은 충돌(장전 후 첫 충돌)로 다르게 동작.
// AGrenadeBase가 이 헤더를 포함하므로 enum도 여기 두어 순환 include를 피한다.
UENUM(BlueprintType)
enum class EGrenadeDetonation : uint8
{
	Fuse   UMETA(DisplayName = "신관(투척용)"),
	Impact UMETA(DisplayName = "충돌(유탄용)")
};

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UGrenadeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="폭발 피해량"))
	float Damage = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="폭발 반경 (cm)"))
	float ExplosionRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="신관 시간 (초). Fuse 모드에서 던진 후 이 시간 뒤 폭발"))
	float FuseTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="지면 반발 계수 (0=흡착, 1=완전 반사)"))
	float Bounciness = 0.3f;

	// --- Detonation ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detonation", meta=(ToolTip="Fuse=신관 시간 후 폭발(투척용), Impact=장전 후 첫 충돌 시 폭발(유탄용)"))
	EGrenadeDetonation DetonationMode = EGrenadeDetonation::Fuse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detonation", meta=(ToolTip="Impact 모드: 발사 후 이 시간(초) 뒤부터 충돌 폭발 활성화 (즉발·자해 방지)", EditCondition="DetonationMode==EGrenadeDetonation::Impact"))
	float ArmingDelay = 0.5f;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="생성 시 1회 재생할 나이아가라 (NS_Bomb_Spawn)"))
	UNiagaraSystem* SpawnVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="비행 중 본체 비주얼 나이아가라 (NS_Bomb_Projectile)"))
	UNiagaraSystem* ProjectileVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="폭발 시 스폰할 나이아가라 (NS_Bomb_Explosion). 폭발 반경에 맞춰 스케일됨"))
	UNiagaraSystem* ExplosionVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="본체 시각 배율 — 생성 FX(User.Scale Overall)·투사체 VFX·충돌 크기에 적용. 폭발 VFX는 폭발 반경 기준이라 영향 없음"))
	float VisualScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ClampMin="0.01", ToolTip="손에 들 때: 생성 FX가 끝나는 즉시 수류탄이 나타나 던질 수 있다. 이 값은 FX가 끝나지 않을 때(루프 등) 대비한 최대 대기 시간(초)"))
	float SpawnFXMaxWait = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="NS_Bomb_Explosion이 제작된 기준 반경(cm). ExplosionRadius와의 비율로 폭발 VFX 스케일 계산"))
	float ExplosionVFXReferenceRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ClampMin="0", ToolTip="폭발 후 본체를 남겨 두는 시간(초). 폭발 VFX가 올라오기 전에 수류탄이 먼저 사라져 보이면 늘린다. 피해·폭발 사운드는 폭발 즉시"))
	float ExplosionBodyLingerTime = 0.1f;

	// --- SFX ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX", meta=(ToolTip="생성 시 재생. 손에 드는 수류탄은 손을 따라다니고, 즉시 발사형(유탄)은 발사 위치에서 재생"))
	USoundBase* SpawnSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX", meta=(ToolTip="손을 떠나는(발사) 순간 재생. 투척은 던지기 몽타주의 Grenade Release 노티파이 시점"))
	USoundBase* ThrowSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX", meta=(ToolTip="바닥·벽에 튕기거나 구를 때(충돌마다) 재생"))
	USoundBase* BounceSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX", meta=(ToolTip="이 속도(cm/s) 미만의 충돌은 튕김 사운드를 내지 않음 — 거의 멈춰 구를 때 소리가 연속으로 쌓이는 것 방지"))
	float BounceSoundMinSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX", meta=(ToolTip="폭발 시 폭발 위치에서 재생"))
	USoundBase* ExplosionSound = nullptr;
};
