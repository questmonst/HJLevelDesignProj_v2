// Copyright Epic Games, Inc. All Rights Reserved.

#include "AkiraEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"

AAkiraEnemy::AAkiraEnemy()
{
	ScriptedMoveSpeed = 600.0f;
	bIsInScriptedMove = false;
}

void AAkiraEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void AAkiraEnemy::StartScriptedMove()
{
	bIsInScriptedMove = true;
	GetCharacterMovement()->MaxWalkSpeed = ScriptedMoveSpeed;
}

void AAkiraEnemy::StopScriptedMove()
{
	bIsInScriptedMove = false;
	// Restore default speed; Blueprint can further adjust
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;
}

void AAkiraEnemy::OnPhaseChange_Implementation(int32 Phase)
{
	// Default: log phase change. Override in Blueprint for phase-specific behaviour.
	UE_LOG(LogTemp, Log, TEXT("AkiraEnemy: Phase changed to %d"), Phase);
}
