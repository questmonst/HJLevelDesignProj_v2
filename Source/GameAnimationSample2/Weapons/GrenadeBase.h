// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeDataAsset.h"	// EGrenadeDetonation, UGrenadeDataAsset
#include "GrenadeBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AGrenadeBase : public AActor
{
	GENERATED_BODY()

public:
	AGrenadeBase();

	// 발사. 아직 손 부착 연출이 끝나지 않았으면(준비 전) 준비 완료 시점에 자동 발사된다.
	void Launch(const FVector& Velocity);

	// 손에 드는 연출 모드. SpawnActorDeferred로 BeginPlay 전에 호출해야 함.
	// true면 생성 FX가 끝난 뒤에야 본체 VFX 부착 + 던지기 가능.
	void SetHeldPresentation(bool bHeld) { bHeldPresentation = bHeld; }

	// 손 부착 연출이 완료돼 던질 수 있는 상태인지
	bool IsReadyToThrow() const { return bReadyToThrow; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="충돌 감지 구체 컴포넌트"))
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="수류탄 메시 컴포넌트"))
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="투사체 이동 컴포넌트"))
	UProjectileMovementComponent* ProjectileMovement;

	// 런타임에 SpawnSystemAttached로 생성·부착됨 (빈 컴포넌트 SetAsset 방식은 렌더 안 되는 이슈가 있어 회피)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="비행 중 본체에 부착되는 나이아가라 VFX 컴포넌트 (NS_Bomb_Projectile)"))
	UNiagaraComponent* ProjectileVFXComponent = nullptr;

	// --- Data ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Data", meta=(ToolTip="할당 시 BeginPlay에서 Stats·Detonation·VFX 값을 이 데이터 에셋으로 덮어씀"))
	UGrenadeDataAsset* GrenadeData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats", meta=(ToolTip="폭발 피해량"))
	float Damage = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats", meta=(ToolTip="폭발 반경 (cm)"))
	float ExplosionRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats", meta=(ToolTip="신관 시간 (초). 던진 후 이 시간이 지나면 폭발"))
	float FuseTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats", meta=(ToolTip="지면 반발 계수 (0=흡착, 1=완전 반사). 낮을수록 굴러가지 않음"))
	float Bounciness = 0.3f;

	// --- Detonation ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Detonation", meta=(ToolTip="Fuse=신관 시간 후 폭발(투척용), Impact=장전 후 첫 충돌 시 폭발(유탄용)"))
	EGrenadeDetonation DetonationMode = EGrenadeDetonation::Fuse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Detonation", meta=(ToolTip="Impact 모드: 발사 후 이 시간(초) 뒤부터 충돌 폭발 활성화 (즉발·자해 방지)", EditCondition="DetonationMode==EGrenadeDetonation::Impact"))
	float ArmingDelay = 0.5f;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|VFX", meta=(ToolTip="생성 시 1회 재생할 나이아가라 (NS_Bomb_Spawn)"))
	UNiagaraSystem* SpawnVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|VFX", meta=(ToolTip="비행 중 본체에 부착되는 나이아가라 (NS_Bomb_Projectile). 본체 비주얼로 사용"))
	UNiagaraSystem* ProjectileVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|VFX", meta=(ToolTip="폭발 시 스폰할 나이아가라 (NS_Bomb_Explosion). 폭발 반경에 맞춰 스케일됨"))
	UNiagaraSystem* ExplosionVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|VFX", meta=(ToolTip="본체(생성·투사체 VFX) 시각 배율. 유탄용은 0.5로 일반 수류탄의 절반 크기"))
	float VisualScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|VFX", meta=(ToolTip="NS_Bomb_Explosion이 제작된 기준 반경(cm). ExplosionRadius와의 비율로 폭발 VFX 스케일 계산"))
	float ExplosionVFXReferenceRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|VFX", meta=(ToolTip="손에 들 때: 생성 FX 재생 후 본체 VFX가 손에 붙기까지의 시간(초). DA에서 조절"))
	float SpawnToProjectileDelay = 0.5f;

	void Explode();

	UFUNCTION(BlueprintNativeEvent, Category = "Grenade")
	void OnExplode();
	virtual void OnExplode_Implementation();

private:
	// Impact 모드: ArmingDelay 후 호출돼 충돌 폭발을 활성화
	void OnArmed();

	// Impact 모드 충돌 폭발 핸들러
	UFUNCTION()
	void OnCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                 FVector NormalImpulse, const FHitResult& Hit);

	// SpawnToProjectileDelay 후 호출돼 본체 VFX 부착 + 던지기 가능 상태로 전환
	void BecomeReady();

	// 실제 발사 처리 (분리 + 이동 활성 + 폭발 타이머)
	void DoLaunch(const FVector& Velocity);

	bool bHeldPresentation = false;	// 손에 드는 연출 모드
	bool bReadyToThrow     = false;	// 손 부착 완료(던지기 가능) 여부
	bool bThrowRequested   = false;	// 준비 전 들어온 던지기 요청 보류
	bool bArmed            = false;	// Impact 모드 장전 완료 여부
	bool bExploded         = false;	// 중복 폭발 방지
	bool bLaunched         = false;	// Launch 1회만 폭발 타이머 시작

	FVector PendingThrowVelocity = FVector::ZeroVector;

	FTimerHandle FuseTimerHandle;
	FTimerHandle ArmingTimerHandle;
	FTimerHandle SpawnReadyTimerHandle;	// 생성 FX 종료 fallback
};
