// Copyright Epic Games, Inc. All Rights Reserved.
#include "BaseAirUnit.h"
#include "GameFramework/CharacterMovementComponent.h"

ABaseAirUnit::ABaseAirUnit()
{
    GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    GetCharacterMovement()->MaxFlySpeed          = FlySpeed;
    GetCharacterMovement()->BrakingDecelerationFlying = 800.f;
    GetCharacterMovement()->GravityScale         = 0.f;
}

void ABaseAirUnit::BeginPlay()
{
    Super::BeginPlay();
    GetCharacterMovement()->MaxFlySpeed = FlySpeed;
}
