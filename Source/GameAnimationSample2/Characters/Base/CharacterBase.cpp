// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterBase.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxHealth = 100.0f;
	CurrentHealth = 100.0f;
	TeamID = 0;
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void ACharacterBase::TakeDamageCustom_Implementation(float Amount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void ACharacterBase::OnDeath_Implementation()
{
	// Default: destroy actor. Override in Blueprint for death animations, ragdoll, etc.
	Destroy();
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                  AController* EventInstigator, AActor* DamageCauser)
{
	float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	TakeDamageCustom(Actual);
	return Actual;
}

void ACharacterBase::Die()
{
	if (bIsDead) return;
	bIsDead = true;
	OnDeath();
}
