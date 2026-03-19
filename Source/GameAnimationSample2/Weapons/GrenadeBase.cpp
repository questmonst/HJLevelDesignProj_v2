// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrenadeBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AGrenadeBase::AGrenadeBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(8.f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed        = 0.f;
	ProjectileMovement->MaxSpeed            = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale   = 1.f;
	ProjectileMovement->bShouldBounce       = true;
	ProjectileMovement->Bounciness          = 0.3f;
}

void AGrenadeBase::BeginPlay()
{
	Super::BeginPlay();
	ProjectileMovement->Bounciness = Bounciness;
	GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &AGrenadeBase::Explode, FuseTime, false);
}

void AGrenadeBase::Launch(const FVector& Velocity)
{
	ProjectileMovement->Velocity = Velocity;
}

void AGrenadeBase::Explode()
{
	TArray<AActor*> Ignored;
	Ignored.Add(this);
	UGameplayStatics::ApplyRadialDamage(
		this, Damage, GetActorLocation(), ExplosionRadius,
		UDamageType::StaticClass(), Ignored, this, GetInstigatorController(), true);

	OnExplode();
}

void AGrenadeBase::OnExplode_Implementation()
{
	Destroy();
}
