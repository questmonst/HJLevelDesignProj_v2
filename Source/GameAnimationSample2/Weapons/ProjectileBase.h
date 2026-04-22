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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Stats")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Stats")
	float InitialSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Stats")
	float GravityScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Stats")
	float LifeSpanSeconds;

	// --- Data ---

	/** 할당 시 BeginPlay에서 Stats·Audio·VFX 값을 덮어씀 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Data")
	UProjectileDataAsset* ProjectileData = nullptr;

	// --- Audio ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Audio")
	USoundBase* HitSound = nullptr;

	// --- VFX ---

	/** 비행 중 투사체에 붙일 나이아가라 시스템 (트레일 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|VFX")
	UNiagaraSystem* FlightVFX = nullptr;

	/** 비행 VFX 스케일 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|VFX")
	float FlightVFXScale = 1.0f;

	/** 충돌 시 히트 위치에 스폰할 나이아가라 시스템 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|VFX")
	UNiagaraSystem* HitVFX = nullptr;

	/** 히트 VFX 스케일 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|VFX")
	float HitVFXScale = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|VFX")
	UNiagaraComponent* FlightVFXComponent;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	           FVector NormalImpulse, const FHitResult& Hit);

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Projectile")
	void OnProjectileHit(AActor* HitActor, const FHitResult& Hit);
	virtual void OnProjectileHit_Implementation(AActor* HitActor, const FHitResult& Hit);

	/** 무기에서 스폰 직후 호출 — BeginPlay 이후에도 속도를 재적용 */
	void OverrideSpeed(float NewSpeed);

	/** 스폰 직후 이 액터를 충돌 무시 목록에 추가 (무기 메시 관통용) */
	void AddIgnoredActor(AActor* Actor);

	float GetDamage() const { return Damage; }
};
