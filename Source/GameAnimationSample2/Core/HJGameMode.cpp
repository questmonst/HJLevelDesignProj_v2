// Copyright Epic Games, Inc. All Rights Reserved.

#include "HJGameMode.h"
#include "PlayerCharacter.h"

AHJGameMode::AHJGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
}
