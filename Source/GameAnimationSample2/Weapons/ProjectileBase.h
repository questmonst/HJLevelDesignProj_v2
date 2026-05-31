// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileDataAsset.h"
#include "ProjectileBase.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta=(ToolTip="충돌 감지 구체 컴포넌트"))
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta=(ToolTip="투사체 이동 컴포넌트"))
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Stats", meta=(ToolTip="투사체 피해량"))
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Stats", meta=(ToolTip="초기 발사 속도 (cm/s)"))
	float InitialSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Stats", meta=(ToolTip="중력 배율 (0=무중력, 1=기본 중력)"))
	float GravityScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Stats", meta=(ToolTip="투사체 수명 (초). 이 시간 후 자동 소멸"))
	float LifeSpanSeconds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Stats", meta=(ToolTip="true면 충돌 시 점 피해 대신 범위 폭발 피해를 적용 (수류탄 등)"))
	bool bRadialDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Stats", meta=(ToolTip="범위 폭발 반경 (cm). bRadialDamage=true일 때만 사용", EditCondition="bRadialDamage"))
	float DamageRadius;

	// --- Data ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Data", meta=(ToolTip="할당 시 BeginPlay에서 Stats·Audio·VFX 값을 덮어씀"))
	UProjectileDataAsset* ProjectileData = nullptr;

	// --- Audio ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Audio", meta=(ToolTip="충돌 시 재생할 사운드 에셋"))
	USoundBase* HitSound = nullptr;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|VFX", meta=(ToolTip="비행 중 투사체에 붙일 나이아가라 시스템 (트레일 등)"))
	UNiagaraSystem* FlightVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|VFX", meta=(ToolTip="비행 VFX 스케일"))
	float FlightVFXScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|VFX", meta=(ToolTip="충돌 시 히트 위치에 스폰할 나이아가라 시스템"))
	UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|VFX", meta=(ToolTip="히트 VFX 스케일"))
	float HitVFXScale = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|VFX", meta=(ToolTip="비행 중 활성화된 나이아가라 VFX 컴포넌트"))
	UNiagaraComponent* FlightVFXComponent;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	           FVector NormalImpulse, const FHitResult& Hit);

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Projectile")
	void OnProjectileHit(AActor* HitActor, const FHitResult& Hit);
	virtual void OnProjectileHit_Implementation(AActor* HitActor, const FHitResult& Hit);

	void OverrideSpeed(float NewSpeed);
	void AddIgnoredActor(AActor* Actor);

	float GetDamage() const { return Damage; }
};
