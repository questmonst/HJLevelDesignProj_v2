// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.generated.h"

/**
 * 캐릭터가 수행할 수 있는 행동 종류.
 * AllowedActions 비트마스크와 함께 사용.
 */
UENUM(BlueprintType, Meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ECharacterAction : uint8
{
	None    = 0x00  UMETA(Hidden),
	Sprint  = 0x01  UMETA(DisplayName="Sprint"),
	Aim     = 0x02  UMETA(DisplayName="Aim"),
	Shoot   = 0x04  UMETA(DisplayName="Shoot"),
	Punch   = 0x08  UMETA(DisplayName="Punch"),
	Landing = 0x10  UMETA(DisplayName="Landing"),
	Jump    = 0x20  UMETA(DisplayName="Jump"),
	Crouch  = 0x40  UMETA(DisplayName="Crouch"),
	Throw   = 0x80  UMETA(DisplayName="Throw"),
};
ENUM_CLASS_FLAGS(ECharacterAction)
