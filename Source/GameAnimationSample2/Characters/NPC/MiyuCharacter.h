// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NPCCharacter.h"
#include "MiyuCharacter.generated.h"

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AMiyuCharacter : public ANPCCharacter
{
	GENERATED_BODY()

public:
	AMiyuCharacter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miyu|Support")
	float SupportFireRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miyu|Support")
	float FireAngleThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miyu|Support")
	float FireCooldown;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Miyu|Support")
	void OnSupportFire();
	virtual void OnSupportFire_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Miyu|Support")
	bool HasLineOfSightToTarget(AActor* Target);
};
