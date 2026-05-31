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
	bRadialDamage   = false;
	DamageRadius    = 300.0f;
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
		bRadialDamage   = ProjectileData->bRadialDamage;
		DamageRadius    = ProjectileData->DamageRadius;
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

	if (bRadialDamage)
	{
		// 폭발 지점 기준 반경 내 모든 액터에 거리 감쇠 피해 (수류탄 등).
		// 충돌 대상(OtherActor)이 벽·바닥이어도 그 지점에서 폭발해야 하므로
		// 점 피해 대신 위치 기반 ApplyRadialDamage를 사용한다.
		UGameplayStatics::ApplyRadialDamage(
			GetWorld(), Damage, Hit.ImpactPoint, DamageRadius,
			nullptr, TArray<AActor*>(), this, GetInstigatorController(),
			false /*bDoFullDamage: false=거리 감쇠*/);
	}
	else
	{
		UGameplayStatics::ApplyPointDamage(
			OtherActor, Damage, GetActorForwardVector(), Hit,
			GetInstigatorController(), this, nullptr);
	}

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
