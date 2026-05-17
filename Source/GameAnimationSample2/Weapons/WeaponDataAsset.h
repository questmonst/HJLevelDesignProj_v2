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
	// --- Mesh ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta=(ToolTip="소켓 기준 무기 메시 회전 오프셋. 원본 메시 방향이 틀어진 경우 여기서 보정"))
	FRotator WeaponMeshRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta=(ToolTip="무기 메시 스케일. 에셋마다 크기가 다를 때 조정"))
	FVector WeaponMeshScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta=(ToolTip="true면 소켓 기준 자동 정렬 사용. 활성화 시 WeaponMeshRotationOffset 무시"))
	bool bUseLeftHandGrip = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta=(ToolTip="그립 정렬 후 추가 위치 오프셋 (actor-local space, cm). 무기별 미세 조정용"))
	FVector WeaponMeshLocationOffset = FVector::ZeroVector;

	// --- Config ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta=(ToolTip="발사 방식. Hitscan=즉시 레이캐스트, Projectile=투사체 스폰"))
	EWeaponFireMode FireMode = EWeaponFireMode::Hitscan;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta=(ToolTip="자동 연사 여부. true면 누르는 동안 계속 발사"))
	bool bIsAutoFire = false;

	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="발사 1회당 피해량"))
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="초당 발사 횟수 (rounds per second)"))
	float FireRate = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="히트스캔 최대 사거리 (cm)"))
	float Range = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="장전 소요 시간 (초)"))
	float ReloadTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="발사 1회당 탄알 수. 1=단발(AR/저격), 8=샷건"))
	int32 PelletCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="탄알 퍼짐 반각 (도). PelletCount > 1일 때 유효. 샷건은 5~10 권장"))
	float PelletSpreadAngle = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="팰릿 간 발사 간격 (초). 0=동시 발사(샷건), >0=순차 발사(점사). bIsAutoFire=false + PelletCount>1 조합으로 점사 구현"))
	float PelletFireInterval = 0.f;

	// --- Ammo ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta=(ToolTip="탄창 용량 (발)"))
	int32 MagSize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta=(ToolTip="초기 예비 탄약 (발)"))
	int32 ReserveAmmo = 90;

	// --- Projectile ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta=(ToolTip="Projectile 모드에서 스폰할 실제 투사체 클래스"))
	TSubclassOf<AProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta=(ToolTip="Hitscan 모드에서 시각 효과용으로 스폰할 투사체 클래스 (충돌·피해 없음)"))
	TSubclassOf<AProjectileBase> CosmeticProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta=(ToolTip="투사체 속도 덮어쓰기 (cm/s). 0이면 투사체 기본값 사용"))
	float ProjectileSpeedOverride = 0.f;

	// --- Spread ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spread", meta=(ToolTip="발사 1회당 추가되는 크로스헤어 퍼짐량"))
	float SpreadPerShot = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spread", meta=(ToolTip="장전 시작 시 추가되는 크로스헤어 퍼짐량"))
	float SpreadReloading = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spread", meta=(ToolTip="초당 크로스헤어 퍼짐 회복량"))
	float SpreadRecoverySpeed = 15.f;

	// --- Recoil ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recoil", meta=(ToolTip="발사 1회당 카메라 위로 올라가는 각도 (도)"))
	float RecoilPitch = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recoil", meta=(ToolTip="발사 1회당 좌우 랜덤 흔들림 최대 각도 (도). 실제 값은 ±이 범위 내 랜덤"))
	float RecoilYawRange = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recoil", meta=(ToolTip="반동 최대 누적 각도 (도). 연사 시 이 값 이상 올라가지 않음"))
	float MaxRecoilPitch = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recoil", meta=(ToolTip="초당 반동 회복 속도 (도/초). 사격 멈추면 이 속도로 원위치"))
	float RecoilRecoverySpeed = 10.f;

	// --- Config ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta=(ToolTip="플레이어 발사 시 카메라 트레이스 시작점 전방 오프셋 (cm). 엄폐물 클리핑 방지용"))
	float TraceStartOffset = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta=(ToolTip="발사 트레이스 디버그 라인 표시 여부"))
	bool bDebugTrace = false;

	// --- Pickup ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta=(ToolTip="픽업 감지 반경 (cm)"))
	float PickupRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta=(ToolTip="HUD 픽업 프롬프트에 표시할 무기 이름"))
	FText WeaponDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta=(ToolTip="주울 때 재생할 SFX"))
	USoundBase* PickupSound = nullptr;

	// --- Animation ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta=(ToolTip="발사 시 캐릭터에 재생할 몽타주 (UpperBody 슬롯). 자동화기는 루프 설정 권장"))
	UAnimMontage* FireMontage = nullptr;

	// --- Audio ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta=(ToolTip="발사 시 재생할 사운드 에셋"))
	USoundBase* FireSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta=(ToolTip="잔탄 없음·장전 중·행동 불가 상태에서 발사 시도 시 재생할 사운드"))
	USoundBase* DryFireSound = nullptr;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="히트스캔 탄착점에 스폰할 나이아가라 시스템"))
	UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="발사 시 총구에 스폰할 나이아가라 시스템 (머즐 플래시)"))
	UNiagaraSystem* MuzzleVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta=(ToolTip="머즐 VFX 스케일"))
	float MuzzleVFXScale = 1.f;
};
