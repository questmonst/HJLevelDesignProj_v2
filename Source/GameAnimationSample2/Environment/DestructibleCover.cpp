// Copyright Epic Games, Inc. All Rights Reserved.

#include "DestructibleCover.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

ADestructibleCover::ADestructibleCover()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));
}

void ADestructibleCover::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

float ADestructibleCover::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDestroyed || DamageAmount <= 0.f) return 0.f;

	const float Applied = FMath::Min(DamageAmount, CurrentHealth);
	CurrentHealth -= Applied;

	if (CurrentHealth <= 0.f)
	{
		bIsDestroyed  = true;
		CurrentHealth = 0.f;
		OnCoverDestroyed();
	}

	return Applied;
}

void ADestructibleCover::OnCoverDestroyed_Implementation()
{
	// 콜리전 즉시 제거 (총알·캐릭터가 통과하도록)
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetVisibility(false);

	// DestroyDelay 후 액터 제거
	SetLifeSpan(DestroyDelay);
}
