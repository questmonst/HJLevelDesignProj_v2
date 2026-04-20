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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Akira|Scripted")
	float ScriptedMoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Akira|Scripted")
	float DefaultMoveSpeed = 400.f;

	UPROPERTY(BlueprintReadOnly, Category = "Akira|Scripted")
	bool bIsInScriptedMove = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Akira|Scripted")
	void StartScriptedMove();

	UFUNCTION(BlueprintCallable, Category = "Akira|Scripted")
	void StopScriptedMove();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Akira|Phase")
	void OnPhaseChange(int32 Phase);
	virtual void OnPhaseChange_Implementation(int32 Phase);
};
