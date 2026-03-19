// Copyright Epic Games, Inc. All Rights Reserved.

#include "NPCCharacter.h"

ANPCCharacter::ANPCCharacter()
{
	DetectionRange = 800.0f;
	bIsAlly        = false;
}

void ANPCCharacter::BeginPlay()
{
	Super::BeginPlay();
}
