// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

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

	FlightVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlightVFX"));
	FlightVFXComponent->SetupAttachment(RootComponent);
	FlightVFXComponent->SetAutoActivate(false);

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

	if (ProjectileData)
	{
		Damage          = ProjectileData->Damage;
		InitialSpeed    = ProjectileData->InitialSpeed;
		GravityScale    = ProjectileData->GravityScale;
		LifeSpanSeconds = ProjectileData->LifeSpanSeconds;
		HitSound        = ProjectileData->HitSound;
		FlightVFX       = ProjectileData->FlightVFX;
		FlightVFXScale  = ProjectileData->FlightVFXScale;
		HitVFX          = ProjectileData->HitVFX;
		HitVFXScale     = ProjectileData->HitVFXScale;

		ProjectileMovement->InitialSpeed           = InitialSpeed;
		ProjectileMovement->MaxSpeed               = InitialSpeed;
		ProjectileMovement->ProjectileGravityScale = GravityScale;
	}

	if (FlightVFX)
	{
		FlightVFXComponent->SetAsset(FlightVFX);
		FlightVFXComponent->SetWorldScale3D(FVector(FlightVFXScale));
		FlightVFXComponent->Activate();
	}

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

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint);
	}

	if (HitVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), HitVFX,
			Hit.ImpactPoint,
			Hit.ImpactNormal.Rotation(),
			FVector(HitVFXScale));
	}

	OnProjectileHit(OtherActor, Hit);
	Destroy();
}

void AProjectileBase::OnProjectileHit_Implementation(AActor* HitActor, const FHitResult& Hit) {}

void AProjectileBase::OverrideSpeed(float NewSpeed)
{
	InitialSpeed = NewSpeed;
	ProjectileMovement->InitialSpeed = NewSpeed;
	ProjectileMovement->MaxSpeed     = NewSpeed;
	ProjectileMovement->Velocity     = GetActorForwardVector() * NewSpeed;
}

void AProjectileBase::AddIgnoredActor(AActor* Actor)
{
	if (Actor)
	{
		CollisionComp->IgnoreActorWhenMoving(Actor, true);
	}
}
