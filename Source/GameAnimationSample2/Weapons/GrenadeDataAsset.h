// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GrenadeDataAsset.generated.h"

class UNiagaraSystem;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="본체(생성·투사체 VFX) 시각 배율. 유탄용은 0.5로 일반 수류탄의 절반 크기"))
	float VisualScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="손에 들 때: 생성 FX 재생 후 본체(투사체) VFX가 손에 붙기까지의 시간(초). 생성 FX 길이에 맞춰 조절"))
	float SpawnToProjectileDelay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="NS_Bomb_Explosion이 제작된 기준 반경(cm). ExplosionRadius와의 비율로 폭발 VFX 스케일 계산"))
	float ExplosionVFXReferenceRadius = 300.f;
};
