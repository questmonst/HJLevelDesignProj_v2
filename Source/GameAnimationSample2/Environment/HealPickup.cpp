// Copyright Epic Games, Inc. All Rights Reserved.

#include "HealPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "PlayerCharacter.h"

AHealPickup::AHealPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(40.f);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->SetCanEverAffectNavigation(false);
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCanEverAffectNavigation(false);
}

void AHealPickup::BeginPlay()
{
	if (HealData)
	{
		HealAmount      = HealData->HealAmount;
		bIgnoreWhenFull = HealData->bIgnoreWhenFull;
		HealEffect      = HealData->HealEffect;
		EffectDuration  = HealData->EffectDuration;
		EffectScale     = HealData->EffectScale;
		PickupSound     = HealData->PickupSound;
	}
	Super::BeginPlay();

	Collision->OnComponentBeginOverlap.AddDynamic(this, &AHealPickup::HandleOverlap);
	SetActorTickEnabled(SpinSpeed > 0.f);
}

void AHealPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Mesh->AddLocalRotation(FRotator(0.f, SpinSpeed * DeltaTime, 0.f));
}

void AHealPickup::HandleOverlap(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(Other);
	if (!Player || Player->IsDead()) return;
	if (bIgnoreWhenFull && Player->GetCurrentHealth() >= Player->GetMaxHealth()) return;

	Player->Heal(HealAmount);

	if (PickupSound) UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());

	// 이펙트는 플레이어에게 붙여 잠깐 보여준다. 아이템은 바로 사라지므로 끄는 건 월드 타이머가 맡는다
	if (HealEffect)
	{
		UNiagaraComponent* FX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			HealEffect, Player->GetRootComponent(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
			FVector(EffectScale), EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None);
		if (FX && EffectDuration > 0.f)
		{
			TWeakObjectPtr<UNiagaraComponent> WeakFX = FX;
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, [WeakFX]()
			{
				if (WeakFX.IsValid()) WeakFX->DestroyComponent();
			}, EffectDuration, false);
		}
	}

	Destroy();
}
