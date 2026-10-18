// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelTriggerVolume.h"
#include "Components/BoxComponent.h"
#include "CheckpointSpotlight.h"
#include "LevelInteraction.h"
#include "PlayerCharacter.h"

ALevelTriggerVolume::ALevelTriggerVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = Box;
	Box->SetBoxExtent(FVector(200.f, 200.f, 150.f));
	Box->SetCollisionProfileName(TEXT("Trigger"));
	Box->SetCanEverAffectNavigation(false);
	Box->ShapeColor = FColor(255, 160, 0);
}

void ALevelTriggerVolume::BeginPlay()
{
	Super::BeginPlay();
	Box->OnComponentBeginOverlap.AddDynamic(this, &ALevelTriggerVolume::HandleBegin);

	if (UCheckpointSubsystem* Checkpoints = GetWorld()->GetSubsystem<UCheckpointSubsystem>())
	{
		Checkpoints->OnPlayerRespawned.AddUObject(this, &ALevelTriggerVolume::HandlePlayerRespawned);
	}
}

void ALevelTriggerVolume::HandleBegin(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!Cast<APlayerCharacter>(Other)) return;
	if (bOnce && bFired) return;

	bFired    = true;
	FiredTime = GetWorld()->GetTimeSeconds();
	LevelSignal::Send(SignalTargets, bSignalValue, this);
	OnFired.Broadcast(this);
}

void ALevelTriggerVolume::HandlePlayerRespawned()
{
	// 체크포인트 이후에 밟은 트리거만 되돌린다 — 체크포인트 이전 구간은 이미 지나온 것
	const UCheckpointSubsystem* Checkpoints = GetWorld()->GetSubsystem<UCheckpointSubsystem>();
	if (bFired && Checkpoints && FiredTime > Checkpoints->GetCheckpointTime())
	{
		bFired = false;
	}
}
