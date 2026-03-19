// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NPCCharacter.h"
#include "AkiraEnemy.generated.h"

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AAkiraEnemy : public ANPCCharacter
{
	GENERATED_BODY()

public:
	AAkiraEnemy();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Akira|Scripted")
	float ScriptedMoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Akira|Scripted")
	bool bIsInScriptedMove;

public:
	UFUNCTION(BlueprintCallable, Category = "Akira|Scripted")
	void StartScriptedMove();

	UFUNCTION(BlueprintCallable, Category = "Akira|Scripted")
	void StopScriptedMove();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Akira|Phase")
	void OnPhaseChange(int32 Phase);
	virtual void OnPhaseChange_Implementation(int32 Phase);
};
