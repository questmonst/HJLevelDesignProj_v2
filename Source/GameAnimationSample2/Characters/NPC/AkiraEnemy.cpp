// Copyright Epic Games, Inc. All Rights Reserved.

#include "AkiraEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"

AAkiraEnemy::AAkiraEnemy()
{
}

void AAkiraEnemy::StartScriptedMove()
{
	bIsInScriptedMove = true;
	GetCharacterMovement()->MaxWalkSpeed = ScriptedMoveSpeed;
}

void AAkiraEnemy::StopScriptedMove()
{
	bIsInScriptedMove = false;
	GetCharacterMovement()->MaxWalkSpeed = DefaultMoveSpeed;
}

void AAkiraEnemy::OnPhaseChange_Implementation(int32 Phase)
{
	UE_LOG(LogTemp, Log, TEXT("AkiraEnemy: Phase changed to %d"), Phase);
}
