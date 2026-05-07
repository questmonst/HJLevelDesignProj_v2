// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

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
		FireMode                = WeaponData->FireMode;
		bIsAutoFire             = WeaponData->bIsAutoFire;
		CosmeticProjectileClass = WeaponData->CosmeticProjectileClass;
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
		SpreadRecoverySpeed     = WeaponData->SpreadRecoverySpeed;
		TraceStartOffset        = WeaponData->TraceStartOffset;
		bDebugTrace             = WeaponData->bDebugTrace;
		HitVFX                  = WeaponData->HitVFX;
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

	const bool bIsPlayer     = Cast<APlayerController>(OwnerCtrl) != nullptr;
	const FVector TraceStart = bIsPlayer ? CamLoc + CamRot.Vector() * TraceStartOffset : CamLoc;
	FVector TraceEnd         = TraceStart + CamRot.Vector() * Range;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

	if (bDebugTrace)
	{
		const FVector DebugEnd = Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;
		UKismetSystemLibrary::DrawDebugLine(this, TraceStart, DebugEnd, FLinearColor::Red, 2.f, 1.f);
		UKismetSystemLibrary::DrawDebugSphere(this, TraceStart, 5.f, 8, FLinearColor::Green, 2.f);
	}

	float ActualDamage = Damage;
	if (CosmeticProjectileClass)
	{
		if (const AProjectileBase* CDO = CosmeticProjectileClass->GetDefaultObject<AProjectileBase>())
			ActualDamage = CDO->GetDamage();
	}

	if (Hit.bBlockingHit)
	{
		if (HitVFX)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

		if (Hit.GetActor())
			UGameplayStatics::ApplyPointDamage(Hit.GetActor(), ActualDamage, CamRot.Vector(), Hit, OwnerCtrl, this, nullptr);
	}

	if (CosmeticProjectileClass)
	{
		FVector MuzzleLoc = WeaponMesh->DoesSocketExist(MuzzleSocketName)
			? WeaponMesh->GetSocketLocation(MuzzleSocketName)
			: GetActorLocation();

		const FVector TargetPoint = Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;
		FRotator SpawnRot = (TargetPoint - MuzzleLoc).Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner      = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		AProjectileBase* Cosmetic = GetWorld()->SpawnActor<AProjectileBase>(
			CosmeticProjectileClass, MuzzleLoc, SpawnRot, SpawnParams);

		if (Cosmetic)
		{
			Cosmetic->SetActorEnableCollision(false);
			if (ProjectileSpeedOverride > 0.f)
				Cosmetic->OverrideSpeed(ProjectileSpeedOverride);

		}
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

	const bool bIsPlayerProj     = Cast<APlayerController>(OwnerCtrl) != nullptr;
	const FVector TraceStartProj = bIsPlayerProj ? CamLoc + CamRot.Vector() * TraceStartOffset : CamLoc;
	FVector TargetPoint          = TraceStartProj + CamRot.Vector() * Range;

	if (bIsPlayerProj)
	{
		FHitResult CamHit;
		FCollisionQueryParams CamParams;
		CamParams.AddIgnoredActor(this);
		CamParams.AddIgnoredActor(GetOwner());
		if (GetWorld()->LineTraceSingleByChannel(CamHit, TraceStartProj, TargetPoint, ECC_Visibility, CamParams))
		{
			TargetPoint = CamHit.ImpactPoint;
		}
	}

	if (bDebugTrace)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, TraceStartProj, 5.f, 8, FLinearColor::Green, 2.f);
		UKismetSystemLibrary::DrawDebugLine(this, TraceStartProj, TargetPoint, FLinearColor::Red, 2.f, 1.f);
	}

	FRotator SpawnRot = (TargetPoint - MuzzleLoc).Rotation();

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
