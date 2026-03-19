// Copyright Epic Games, Inc. All Rights Reserved.

#include "MiyuCharacter.h"
#include "Engine/World.h"

AMiyuCharacter::AMiyuCharacter()
{
	SupportFireRange   = 1200.0f;
	FireAngleThreshold = 30.0f;
	FireCooldown       = 2.0f;
}

void AMiyuCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMiyuCharacter::OnSupportFire_Implementation()
{
	// Default: fire a projectile toward the target. Override in Blueprint.
}

bool AMiyuCharacter::HasLineOfSightToTarget(AActor* Target)
{
	if (!Target || !GetWorld())
	{
		return false;
	}

	FVector Start = GetActorLocation();
	FVector End   = Target->GetActorLocation();

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Target);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	// No blocking hit between Miyu and the target means line-of-sight is clear
	return !bHit;
}
