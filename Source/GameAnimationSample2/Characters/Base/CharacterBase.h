// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "CharacterBase.generated.h"

UCLASS(Blueprintable, BlueprintType, meta=(PrioritizeCategories="Character|Stats"))
class GAMEANIMATIONSAMPLE2_API ACharacterBase : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ACharacterBase();

protected:
	virtual void BeginPlay() override;

	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats", meta=(ToolTip="최대 체력"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats", meta=(ToolTip="현재 체력. BeginPlay에서 MaxHealth로 초기화"))
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats", meta=(ToolTip="팀 ID. 0=플레이어·아군, 1=적. AIPerception 팀 감지에 사용"))
	int32 TeamID;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Stats", meta=(ToolTip="사망 여부. true면 OnDeath 이후 상태"))
	bool bIsDead = false;

	// --- Internal ---

	void Die();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	                         AController* EventInstigator, AActor* DamageCauser) override;

public:
	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(TeamID); }

	// --- BlueprintNativeEvent ---

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character|Combat")
	void OnDeath();
	virtual void OnDeath_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character|Combat")
	void TakeDamageCustom(float Amount);
	virtual void TakeDamageCustom_Implementation(float Amount);
};
