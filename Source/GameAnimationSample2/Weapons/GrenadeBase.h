// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AGrenadeBase : public AActor
{
	GENERATED_BODY()

public:
	AGrenadeBase();

	void Launch(const FVector& Velocity);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade")
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats")
	float Damage = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats")
	float ExplosionRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats")
	float FuseTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats")
	float Bounciness = 0.3f;

	void Explode();

	UFUNCTION(BlueprintNativeEvent, Category = "Grenade")
	void OnExplode();
	virtual void OnExplode_Implementation();

private:
	FTimerHandle FuseTimerHandle;
};
