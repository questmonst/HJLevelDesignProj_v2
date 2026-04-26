// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

UCLASS(Blueprintable, BlueprintType, meta=(PrioritizeCategories="Character|Stats"))
class GAMEANIMATIONSAMPLE2_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ACharacterBase();

protected:
	virtual void BeginPlay() override;

	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats")
	int32 TeamID;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Stats")
	bool bIsDead = false;

	// --- Internal ---

	void Die();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	                         AController* EventInstigator, AActor* DamageCauser) override;

public:
	// --- BlueprintNativeEvent ---

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character|Combat")
	void OnDeath();
	virtual void OnDeath_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character|Combat")
	void TakeDamageCustom(float Amount);
	virtual void TakeDamageCustom_Implementation(float Amount);
};
