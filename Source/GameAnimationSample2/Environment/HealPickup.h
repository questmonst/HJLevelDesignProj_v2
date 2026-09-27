// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataAsset.h"
#include "HealPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraSystem;
class USoundBase;

// 회복 아이템 수치 (DA_HealPickup)
UCLASS(BlueprintType)
class GAMEANIMATIONSAMPLE2_API UHealPickupData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ClampMin="0", ToolTip="회복량"))
	float HealAmount = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ToolTip="체력이 가득이면 먹지 않고 남겨 둔다"))
	bool bIgnoreWhenFull = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ToolTip="먹었을 때 플레이어에게 잠깐 붙는 이펙트"))
	UNiagaraSystem* HealEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ClampMin="0", ToolTip="이펙트가 붙어 있는 시간(초)"))
	float EffectDuration = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ToolTip="이펙트 크기"))
	float EffectScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ToolTip="먹었을 때 소리"))
	USoundBase* PickupSound = nullptr;
};

// 회복 아이템 — 플레이어가 닿으면 체력을 채우고 사라진다. 이펙트는 플레이어에게 잠깐 붙는다
UCLASS(Blueprintable)
class GAMEANIMATIONSAMPLE2_API AHealPickup : public AActor
{
	GENERATED_BODY()

public:
	AHealPickup();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heal")
	USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heal")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ToolTip="수치 DA (DA_HealPickup). 지정하면 아래 값을 덮어쓴다"))
	UHealPickupData* HealData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ClampMin="0", ToolTip="회복량. HealData에서 설정"))
	float HealAmount = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ToolTip="체력이 가득이면 먹지 않는다. HealData에서 설정"))
	bool bIgnoreWhenFull = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ToolTip="플레이어에게 붙는 이펙트. HealData에서 설정"))
	UNiagaraSystem* HealEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ClampMin="0", ToolTip="이펙트 유지 시간(초). HealData에서 설정"))
	float EffectDuration = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ToolTip="이펙트 크기. HealData에서 설정"))
	float EffectScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ToolTip="먹었을 때 소리. HealData에서 설정"))
	USoundBase* PickupSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta=(ClampMin="0", ToolTip="제자리 회전 속도(도/초). 0이면 멈춤"))
	float SpinSpeed = 90.f;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);
};
