// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IDestructible.h"
#include "DestructibleCover.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

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
