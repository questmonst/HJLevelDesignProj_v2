// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	FireMode       = EWeaponFireMode::Hitscan;
	bIsAutoFire    = false;
	MuzzleSocketName = TEXT("Muzzle");

	Damage      = 20.0f;
	FireRate    = 10.0f;
	Range       = 5000.0f;
	ReloadTime  = 2.0f;

	MagSize     = 30;
	CurrentAmmo = 30;
	ReserveAmmo = 90;

	bIsReloading = false;
}

bool AWeaponBase::CanFire() const
{
	return !bIsReloading && CurrentAmmo > 0;
}

void AWeaponBase::StartFire()
{
	Fire();

	if (bIsAutoFire && CanFire())
	{
		float Interval = 1.0f / FMath::Max(FireRate, 0.1f);
		GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &AWeaponBase::Fire, Interval, true);
	}
}

void AWeaponBase::StopFire()
{
	GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
}

void AWeaponBase::Fire()
{
	if (!CanFire()) return;

	CurrentAmmo--;

	switch (FireMode)
	{
	case EWeaponFireMode::Hitscan:    HitscanFire();    break;
	case EWeaponFireMode::Projectile: ProjectileFire(); break;
	}

	if (CurrentAmmo == 0 && ReserveAmmo > 0)
	{
		Reload();
	}
}

void AWeaponBase::HitscanFire()
{
	APawn* OwnerPawn           = Cast<APawn>(GetOwner());
	AController* OwnerCtrl     = OwnerPawn ? OwnerPawn->GetController() : nullptr;

	FVector  CamLoc;
	FRotator CamRot;

	if (APlayerController* PC = Cast<APlayerController>(OwnerCtrl))
	{
		PC->GetPlayerViewPoint(CamLoc, CamRot);
	}
	else if (OwnerPawn)
	{
		CamLoc = OwnerPawn->GetActorLocation();
		CamRot = OwnerPawn->GetActorRotation();
	}
	else return;

	FVector TraceEnd = CamLoc + CamRot.Vector() * Range;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, TraceEnd, ECC_Visibility, Params);

	if (Hit.bBlockingHit && Hit.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(
			Hit.GetActor(), Damage, CamRot.Vector(), Hit,
			OwnerCtrl, this, nullptr);
	}

	OnFire(Hit);
}

void AWeaponBase::ProjectileFire()
{
	if (!ProjectileClass) return;

	FVector MuzzleLoc = WeaponMesh->DoesSocketExist(MuzzleSocketName)
		? WeaponMesh->GetSocketLocation(MuzzleSocketName)
		: GetActorLocation();

	APawn* OwnerPawn       = Cast<APawn>(GetOwner());
	AController* OwnerCtrl = OwnerPawn ? OwnerPawn->GetController() : nullptr;

	FVector  CamLoc;
	FRotator CamRot;

	if (APlayerController* PC = Cast<APlayerController>(OwnerCtrl))
	{
		PC->GetPlayerViewPoint(CamLoc, CamRot);
	}
	else if (OwnerPawn)
	{
		CamLoc = OwnerPawn->GetActorLocation();
		CamRot = OwnerPawn->GetActorRotation();
	}
	else return;

	FVector  TargetPoint = CamLoc + CamRot.Vector() * Range;
	FRotator SpawnRot    = (TargetPoint - MuzzleLoc).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner      = GetOwner();
	SpawnParams.Instigator = OwnerPawn;

	GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, MuzzleLoc, SpawnRot, SpawnParams);

	FHitResult DummyHit;
	OnFire(DummyHit);
}

void AWeaponBase::Reload()
{
	if (bIsReloading || CurrentAmmo == MagSize || ReserveAmmo == 0) return;

	bIsReloading = true;
	StopFire();
	OnReloadStart();

	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AWeaponBase::FinishReload, ReloadTime, false);
}

void AWeaponBase::FinishReload()
{
	int32 Needed   = MagSize - CurrentAmmo;
	int32 ToReload = FMath::Min(Needed, ReserveAmmo);
	CurrentAmmo  += ToReload;
	ReserveAmmo  -= ToReload;
	bIsReloading   = false;
	OnReloadFinish();
}

void AWeaponBase::OnFire_Implementation(const FHitResult& HitResult)        {}
void AWeaponBase::OnReloadStart_Implementation()                            {}
void AWeaponBase::OnReloadFinish_Implementation()                           {}
