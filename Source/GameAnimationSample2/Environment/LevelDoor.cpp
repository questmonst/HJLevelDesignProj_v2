// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "PlayerCharacter.h"

ALevelDoor::ALevelDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;   // 움직일 때만 켠다

	DoorLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorLeft"));
	DoorLeft->SetupAttachment(Root);

	DoorRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorRight"));
	DoorRight->SetupAttachment(Root);
}

void ALevelDoor::BeginPlay()
{
	LeftClosed  = DoorLeft->GetRelativeLocation();
	RightClosed = DoorRight->GetRelativeLocation();
	Super::BeginPlay();
}

void ALevelDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float Target = bWantsOpen ? 1.f : 0.f;
	OpenAlpha = FMath::FInterpConstantTo(OpenAlpha, Target, DeltaTime, 1.f / OpenTime);

	const float Ease   = FMath::InterpEaseInOut(0.f, 1.f, OpenAlpha, 2.f);
	const FVector Offset(0.f, OpenDistance * Ease, 0.f);
	DoorLeft->SetRelativeLocation(LeftClosed - Offset);
	DoorRight->SetRelativeLocation(RightClosed + Offset);

	if (FMath::IsNearlyEqual(OpenAlpha, Target)) SetActorTickEnabled(false);
}

void ALevelDoor::OpenDoor()
{
	GetWorldTimerManager().ClearTimer(CloseTimerHandle);
	if (bWantsOpen) return;
	bWantsOpen = true;
	SetActorTickEnabled(true);
	if (OpenSound) UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());
	OnDoorStateChanged.Broadcast(this, true);
}

void ALevelDoor::CloseDoor()
{
	GetWorldTimerManager().ClearTimer(CloseTimerHandle);
	if (!bWantsOpen) return;
	bWantsOpen = false;
	SetActorTickEnabled(true);
	if (CloseSound) UGameplayStatics::PlaySoundAtLocation(this, CloseSound, GetActorLocation());
	OnDoorStateChanged.Broadcast(this, false);
}

bool ALevelDoor::CanInteract(const APlayerCharacter* Player) const
{
	return OpenMode == EDoorOpenMode::Interact && bInteractable && !bWantsOpen;
}

void ALevelDoor::Interact(APlayerCharacter* Player)
{
	if (CanInteract(Player)) OpenDoor();
}

void ALevelDoor::ReceiveLevelSignal(bool bOn, AActor* Source)
{
	if (SignalAction == EDoorSignalAction::Open)
	{
		if (bOn) OpenDoor(); else CloseDoor();
		return;
	}
	SetInteractable(bOn);
}

void ALevelDoor::OnPlayerEnterZone(APlayerCharacter* Player)
{
	if (OpenMode == EDoorOpenMode::Interact)
	{
		Super::OnPlayerEnterZone(Player);   // E 안내 후보로 등록
		GetWorldTimerManager().ClearTimer(CloseTimerHandle);
		return;
	}
	if (bInteractable) OpenDoor();
}

void ALevelDoor::OnPlayerLeaveZone(APlayerCharacter* Player)
{
	if (OpenMode == EDoorOpenMode::Interact)
	{
		Super::OnPlayerLeaveZone(Player);
		if (!bCloseWhenPlayerLeaves) return;
	}
	if (IsPlayerInZone()) return;

	if (CloseDelay > 0.f)
		GetWorldTimerManager().SetTimer(CloseTimerHandle, this, &ALevelDoor::CloseDoor, CloseDelay, false);
	else
		CloseDoor();
}

void ALevelDoor::OnInteractableStateChanged()
{
	if (!bInteractable)
	{
		if (bCloseWhenLocked) CloseDoor();
		return;
	}
	// 자동문 앞에 서 있는데 청색이 되면 바로 연다
	if (OpenMode == EDoorOpenMode::Auto && IsPlayerInZone()) OpenDoor();
}
