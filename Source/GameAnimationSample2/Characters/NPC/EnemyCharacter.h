// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NPCCharacter.h"
#include "EnemyCharacter.generated.h"

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AEnemyCharacter : public ANPCCharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float AttackCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Patrol")
	float PatrolRadius;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Enemy|AI")
	void OnDetectPlayer();
	virtual void OnDetectPlayer_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Enemy|Combat")
	void OnAttack();
	virtual void OnAttack_Implementation();
};
