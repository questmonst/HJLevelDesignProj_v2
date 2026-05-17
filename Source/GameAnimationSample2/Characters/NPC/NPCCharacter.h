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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection", meta=(ToolTip="감지 범위 (cm). 이 범위 내 플레이어를 인지"))
	float DetectionRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Faction", meta=(ToolTip="아군 여부. true면 플레이어 편 (TeamID 0)"))
	bool bIsAlly;
};
