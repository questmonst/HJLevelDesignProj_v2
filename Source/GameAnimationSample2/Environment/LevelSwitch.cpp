// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelSwitch.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ALevelSwitch::ALevelSwitch()
{
	SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));
	SwitchMesh->SetupAttachment(Root);
}

void ALevelSwitch::Interact(APlayerCharacter* Player)
{
	if (!CanInteract(Player)) return;

	if (UseSound) UGameplayStatics::PlaySoundAtLocation(this, UseSound, GetActorLocation());

	if (bLockAfterUse)
	{
		SetInteractable(false);
		if (ReuseCooldown > 0.f)
		{
			GetWorldTimerManager().SetTimer(ReuseTimerHandle, this, &ALevelSwitch::Unlock, ReuseCooldown, false);
		}
	}

	LevelSignal::Send(SignalTargets, bSignalValue, this);
	OnSwitchUsed.Broadcast(this);
}
