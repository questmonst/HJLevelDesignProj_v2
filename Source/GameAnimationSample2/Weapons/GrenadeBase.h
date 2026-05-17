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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="충돌 감지 구체 컴포넌트"))
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="수류탄 메시 컴포넌트"))
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="투사체 이동 컴포넌트"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats", meta=(ToolTip="폭발 피해량"))
	float Damage = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats", meta=(ToolTip="폭발 반경 (cm)"))
	float ExplosionRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats", meta=(ToolTip="신관 시간 (초). 던진 후 이 시간이 지나면 폭발"))
	float FuseTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Stats", meta=(ToolTip="지면 반발 계수 (0=흡착, 1=완전 반사). 낮을수록 굴러가지 않음"))
	float Bounciness = 0.3f;

	void Explode();

	UFUNCTION(BlueprintNativeEvent, Category = "Grenade")
	void OnExplode();
	virtual void OnExplode_Implementation();

private:
	FTimerHandle FuseTimerHandle;
};
