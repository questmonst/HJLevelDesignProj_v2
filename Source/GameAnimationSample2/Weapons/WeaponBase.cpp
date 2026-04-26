// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerCharacter.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	FireMode       = EWeaponFireMode::Hitscan;
	bIsAutoFire    = false;
	MuzzleSocketName = TEXT("MuzzleSocket");

	Damage      = 20.0f;
	FireRate    = 10.0f;
	Range       = 5000.0f;
	ReloadTime  = 2.0f;

	MagSize     = 30;
	CurrentAmmo = 30;
	ReserveAmmo = 90;

	bIsReloading = false;
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponData)
	{
		Damage                  = WeaponData->Damage;
		FireRate                = WeaponData->FireRate;
		Range                   = WeaponData->Range;
		ReloadTime              = WeaponData->ReloadTime;
		MagSize                 = WeaponData->MagSize;
		CurrentAmmo             = WeaponData->MagSize;
		ReserveAmmo             = WeaponData->ReserveAmmo;
		ProjectileSpeedOverride = WeaponData->ProjectileSpeedOverride;
		SpreadPerShot           = WeaponData->SpreadPerShot;
		SpreadReloading         = WeaponData->SpreadReloading;
		FireSound               = WeaponData->FireSound;
		MuzzleVFX               = WeaponData->MuzzleVFX;
		MuzzleVFXScale          = WeaponData->MuzzleVFXScale;
	}
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

	FTransform MuzzleTransform = WeaponMesh->DoesSocketExist(MuzzleSocketName)
		? WeaponMesh->GetSocketTransform(MuzzleSocketName)
		: GetActorTransform();

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleTransform.GetLocation());
	}

	if (MuzzleVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), MuzzleVFX,
			MuzzleTransform.GetLocation(),
			MuzzleTransform.GetRotation().Rotator(),
			FVector(MuzzleVFXScale));
	}

	switch (FireMode)
	{
	case EWeaponFireMode::Hitscan:    HitscanFire();    break;
	case EWeaponFireMode::Projectile: ProjectileFire(); break;
	}

	if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwner()))
	{
		PC->AddCrosshairSpread(SpreadPerShot);
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

	AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, MuzzleLoc, SpawnRot, SpawnParams);

	if (Projectile)
	{
		Projectile->AddIgnoredActor(this);
		Projectile->AddIgnoredActor(GetOwner());

		if (ProjectileSpeedOverride > 0.f)
		{
			Projectile->OverrideSpeed(ProjectileSpeedOverride);
		}
	}

	FHitResult DummyHit;
	OnFire(DummyHit);
}

void AWeaponBase::Reload()
{
	if (bIsReloading || CurrentAmmo == MagSize || ReserveAmmo == 0) return;

	bIsReloading = true;
	StopFire();
	OnReloadStart();

	if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwner()))
	{
		PC->AddCrosshairSpread(SpreadReloading);
	}

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
