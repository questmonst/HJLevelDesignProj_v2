// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleCover.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ADestructibleCover : public AActor
{
	GENERATED_BODY()

public:
	ADestructibleCover();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	UStaticMeshComponent* MeshComp;

	// --- 체력 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Health")
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Cover|Health")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Cover|Health")
	bool bIsDestroyed = false;

	/** 파괴 후 액터 제거까지 대기 시간 (Chaos 연출 재생 시간) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover|Health")
	float DestroyDelay = 2.f;

	virtual void BeginPlay() override;

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	/** 파괴 시 호출 — BP에서 Chaos GC 스폰·이펙트 처리 */
	UFUNCTION(BlueprintNativeEvent, Category = "Cover")
	void OnCoverDestroyed();
	virtual void OnCoverDestroyed_Implementation();

public:
	UFUNCTION(BlueprintPure, Category = "Cover|Health")
	float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Cover|Health")
	bool IsDestroyed() const { return bIsDestroyed; }
};
