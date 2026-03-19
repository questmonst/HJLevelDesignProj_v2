// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	Damage          = 20.0f;
	InitialSpeed    = 3000.0f;
	GravityScale    = 0.1f;
	LifeSpanSeconds = 3.0f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed     = InitialSpeed;
	ProjectileMovement->ProjectileGravityScale = GravityScale;

	SetLifeSpan(LifeSpanSeconds);
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                             const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner()) return;

	UGameplayStatics::ApplyPointDamage(
		OtherActor, Damage, GetActorForwardVector(), Hit,
		GetInstigatorController(), this, nullptr);

	OnProjectileHit(OtherActor, Hit);
	Destroy();
}

void AProjectileBase::OnProjectileHit_Implementation(AActor* HitActor, const FHitResult& Hit)
{
	// Override in Blueprint for effects (explosion, sound, decal, etc.)
}
