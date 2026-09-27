// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IDestructible.h"
#include "DestructibleCover.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UGeometryCollection;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ADestructibleCover : public AActor, public IDestructibleObject
{
	GENERATED_BODY()

public:
	ADestructibleCover();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta=(ToolTip="엄폐물 스태틱 메시 컴포넌트"))
	UStaticMeshComponent* MeshComp;

	// --- 체력 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Health", meta=(ToolTip="최대 체력"))
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Cover|Health", meta=(ToolTip="현재 체력"))
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Cover|Health", meta=(ToolTip="파괴 여부. true면 이미 파괴된 상태"))
	bool bIsDestroyed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Health", meta=(ToolTip="파괴 후 액터 제거까지 대기 시간 (초). Chaos 연출 재생 시간"))
	float DestroyDelay = 2.f;

	// --- 깨짐 연출 ---
	// 메시를 Fracture 모드로 쪼갠 Geometry Collection을 지정하면, 파괴 순간 그 자리에 파편을 스폰해
	// 맞은 지점에서 바깥으로 흩어지게 한다. 비우면 예전처럼 그냥 사라진다

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Break", meta=(ToolTip="파괴 시 스폰할 파편 (이 엄폐물 메시를 Fracture 모드로 쪼갠 Geometry Collection). 비우면 그냥 사라짐"))
	UGeometryCollection* FractureCollection = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Break", meta=(ClampMin="0", ToolTip="파편을 흩뿌리는 속도(cm/s). 맞은 지점에서 바깥쪽으로"))
	float BreakImpulse = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Break", meta=(ClampMin="0", ToolTip="흩뿌림이 미치는 반경(cm)"))
	float BreakRadius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Break", meta=(ClampMin="0", ToolTip="파편이 남아 있는 시간(초). 0이면 계속 남음"))
	float DebrisLifeSpan = 6.f;

	// 마지막으로 맞은 지점 — 파편이 이 점에서 바깥으로 튄다
	FVector LastHitLocation = FVector::ZeroVector;

	void SpawnDebris();

	virtual void BeginPlay() override;

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Cover")
	void OnCoverDestroyed();
	virtual void OnCoverDestroyed_Implementation();

public:
	UFUNCTION(BlueprintPure, Category = "Cover|Health")
	float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Cover|Health")
	bool IsDestroyed() const { return bIsDestroyed; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Destructible")
	void DestroyByPunch(AActor* PunchInstigator);
	virtual void DestroyByPunch_Implementation(AActor* PunchInstigator) override;
};
