// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelInteractableBase.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PlayerCharacter.h"

ALevelInteractableBase::ALevelInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	InteractZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractZone"));
	InteractZone->SetupAttachment(Root);
	InteractZone->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	InteractZone->SetCollisionProfileName(TEXT("Trigger"));
	InteractZone->SetCanEverAffectNavigation(false);

	StatusLampMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatusLampMesh"));
	StatusLampMesh->SetupAttachment(Root);
	StatusLampMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StatusLampMesh->SetCanEverAffectNavigation(false);

	StatusLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StatusLight"));
	StatusLight->SetupAttachment(StatusLampMesh);
	StatusLight->SetIntensity(800.f);
	StatusLight->SetAttenuationRadius(250.f);
	StatusLight->SetCastShadows(false);
}

void ALevelInteractableBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	// 에디터에서 배치할 때도 청/적이 보이게 (머티리얼 색은 게임 시작 후 적용)
	StatusLight->SetVisibility(bUseStatusLight);
	StatusLampMesh->SetVisibility(bUseStatusLight);
	StatusLight->SetLightColor(bInteractable ? InteractableColor : LockedColor);
}

void ALevelInteractableBase::BeginPlay()
{
	Super::BeginPlay();

	LampMaterials.Reset();
	if (bUseStatusLight && !StatusColorParam.IsNone())
	{
		for (int32 i = 0; i < StatusLampMesh->GetNumMaterials(); ++i)
		{
			if (UMaterialInstanceDynamic* MID = StatusLampMesh->CreateAndSetMaterialInstanceDynamic(i))
			{
				LampMaterials.Add(MID);
			}
		}
	}

	InteractZone->OnComponentBeginOverlap.AddDynamic(this, &ALevelInteractableBase::HandleZoneBegin);
	InteractZone->OnComponentEndOverlap.AddDynamic(this, &ALevelInteractableBase::HandleZoneEnd);

	ApplyStatusVisuals();

	// 게임 시작 시 이미 영역 안에 있던 플레이어도 잡는다
	TArray<AActor*> Overlapping;
	InteractZone->GetOverlappingActors(Overlapping, APlayerCharacter::StaticClass());
	for (AActor* Actor : Overlapping)
	{
		OnPlayerEnterZone(Cast<APlayerCharacter>(Actor));
	}
}

void ALevelInteractableBase::SetInteractable(bool bNewInteractable)
{
	if (bInteractable == bNewInteractable) return;
	bInteractable = bNewInteractable;
	ApplyStatusVisuals();
	OnInteractableStateChanged();
	OnInteractableChanged.Broadcast(this, bInteractable);
}

void ALevelInteractableBase::ReceiveLevelSignal(bool bOn, AActor* Source)
{
	SetInteractable(bOn);
}

void ALevelInteractableBase::ApplyStatusVisuals()
{
	StatusLight->SetVisibility(bUseStatusLight);
	StatusLampMesh->SetVisibility(bUseStatusLight);
	if (!bUseStatusLight) return;

	const FLinearColor Color = bInteractable ? InteractableColor : LockedColor;
	StatusLight->SetLightColor(Color);
	for (UMaterialInstanceDynamic* MID : LampMaterials)
	{
		if (MID) MID->SetVectorParameterValue(StatusColorParam, Color);
	}
}

void ALevelInteractableBase::HandleZoneBegin(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(Other)) OnPlayerEnterZone(Player);
}

void ALevelInteractableBase::HandleZoneEnd(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(Other)) OnPlayerLeaveZone(Player);
}

void ALevelInteractableBase::OnPlayerEnterZone(APlayerCharacter* Player)
{
	if (Player) Player->AddInteractCandidate(this);
}

void ALevelInteractableBase::OnPlayerLeaveZone(APlayerCharacter* Player)
{
	if (Player) Player->RemoveInteractCandidate(this);
}

bool ALevelInteractableBase::IsPlayerInZone() const
{
	TArray<AActor*> Overlapping;
	InteractZone->GetOverlappingActors(Overlapping, APlayerCharacter::StaticClass());
	return Overlapping.Num() > 0;
}
