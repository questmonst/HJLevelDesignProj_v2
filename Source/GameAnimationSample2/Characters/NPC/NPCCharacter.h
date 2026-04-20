// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "NPCCharacter.generated.h"

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ANPCCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	ANPCCharacter();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
	float DetectionRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Faction")
	bool bIsAlly;
};
