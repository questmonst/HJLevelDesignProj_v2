// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyCharacter.h"

AEnemyCharacter::AEnemyCharacter()
{
	AttackDamage   = 20.0f;
	AttackRange    = 150.0f;
	AttackCooldown = 1.5f;
	PatrolRadius   = 500.0f;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyCharacter::OnDetectPlayer_Implementation()
{
	// Default: begin pursuit. Override in Blueprint or derived C++ class.
}

void AEnemyCharacter::OnAttack_Implementation()
{
	// Default: deal damage to closest detected target. Override in Blueprint.
}
