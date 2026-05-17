// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = DefaultRoot;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(RootComponent);
	PickupSphere->SetSphereRadius(100.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnPickupSphereBeginOverlap);
	PickupSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponBase::OnPickupSphereEndOverlap);

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
		// --- Mesh ---
		WeaponMeshRotationOffset = WeaponData->WeaponMeshRotationOffset;
		WeaponMeshScale          = WeaponData->WeaponMeshScale;
		bUseLeftHandGrip         = WeaponData->bUseLeftHandGrip;
		WeaponMeshLocationOffset = WeaponData->WeaponMeshLocationOffset;
		PickupRadius             = WeaponData->PickupRadius;
		// --- Config ---
		FireMode                = WeaponData->FireMode;
		bIsAutoFire             = WeaponData->bIsAutoFire;
		bShowArcTrajectory      = WeaponData->bShowArcTrajectory;
		bAmmoPerPellet          = WeaponData->bAmmoPerPellet;
		ProjectileClass         = WeaponData->ProjectileClass;
		CosmeticProjectileClass = WeaponData->CosmeticProjectileClass;
		// --- Stats ---
		Damage                  = WeaponData->Damage;
		FireRate                = WeaponData->FireRate;
		Range                   = WeaponData->Range;
		ReloadTime              = WeaponData->ReloadTime;
		MagSize                 = WeaponData->MagSize;
		CurrentAmmo             = WeaponData->MagSize;
		ReserveAmmo             = WeaponData->ReserveAmmo;
		PelletCount             = WeaponData->PelletCount;
		PelletSpreadAngle       = WeaponData->PelletSpreadAngle;
		PelletFireInterval      = WeaponData->PelletFireInterval;
		ProjectileSpeedOverride = WeaponData->ProjectileSpeedOverride;
		// --- Spread ---
		SpreadPerShot           = WeaponData->SpreadPerShot;
		SpreadReloading         = WeaponData->SpreadReloading;
		SpreadRecoverySpeed     = WeaponData->SpreadRecoverySpeed;
		TraceStartOffset        = WeaponData->TraceStartOffset;
		bDebugTrace             = WeaponData->bDebugTrace;
		// --- Recoil ---
		RecoilPitch             = WeaponData->RecoilPitch;
		RecoilYawRange          = WeaponData->RecoilYawRange;
		MaxRecoilPitch          = WeaponData->MaxRecoilPitch;
		RecoilRecoverySpeed     = WeaponData->RecoilRecoverySpeed;
		// --- Audio / VFX ---
		FireSound               = WeaponData->FireSound;
		DryFireSound            = WeaponData->DryFireSound;
		HitVFX                  = WeaponData->HitVFX;
		MuzzleVFX               = WeaponData->MuzzleVFX;
		MuzzleVFXScale          = WeaponData->MuzzleVFXScale;
		// --- Animation ---
		FireMontage             = WeaponData->FireMontage;
	}

	RefreshMeshTransform();
	PickupSphere->SetSphereRadius(PickupRadius);

	// Owner 있으면(SpawnActor로 생성) 인벤토리 무기로 간주해 드롭 상태 해제
	if (GetOwner())
		bIsDropped = false;

	PickupSphere->SetCollisionEnabled(bIsDropped ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

bool AWeaponBase::CanFire() const
{
	return !bIsReloading && CurrentAmmo > 0;
}

void AWeaponBase::StartFire()
{
	if (!CanFire())
	{
		if (DryFireSound)
			UGameplayStatics::PlaySoundAtLocation(this, DryFireSound, GetActorLocation());
		return;
	}

	if (!bIsAutoFire)
	{
		if (bFireCooldown)
		{
			if (DryFireSound)
				UGameplayStatics::PlaySoundAtLocation(this, DryFireSound, GetActorLocation());
			return;
		}

		if (PelletFireInterval > 0.f && PelletCount > 1)
		{
			// 순차 발사(점사): 첫 발 즉시, 이후 PelletFireInterval 간격으로 PelletCount발
			bFireCooldown      = true;
			CurrentPelletShot  = 0;
			FireNextPelletInBurst();
		}
		else
		{
			Fire();
			if (CanFire())
			{
				bFireCooldown = true;
				const float Interval = 1.0f / FMath::Max(FireRate, 0.1f);
				GetWorldTimerManager().SetTimer(SemiFireTimerHandle, this, &AWeaponBase::ResetSemiFireCooldown, Interval, false);
			}
		}
		return;
	}

	Fire();
	if (CanFire())
	{
		const float Interval = 1.0f / FMath::Max(FireRate, 0.1f);
		GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &AWeaponBase::Fire, Interval, true);
	}
}

void AWeaponBase::StopFire()
{
	GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
}

void AWeaponBase::FireNextPelletInBurst()
{
	if (!CanFire())
	{
		CurrentPelletShot = 0;
		ResetSemiFireCooldown();
		return;
	}

	const int32 BurstTotal = PelletCount; // 저장: Fire() 내부에서 PelletCount 읽음

	// PelletCount를 1로 임시 설정 → Fire()가 단발로 동작
	PelletCount = 1;
	Fire();
	PelletCount = BurstTotal;

	++CurrentPelletShot;

	if (CurrentPelletShot < BurstTotal && CanFire())
	{
		GetWorldTimerManager().SetTimer(PelletFireTimerHandle, this,
			&AWeaponBase::FireNextPelletInBurst, PelletFireInterval, false);
	}
	else
	{
		CurrentPelletShot = 0;
		const float Interval = 1.f / FMath::Max(FireRate, 0.1f);
		GetWorldTimerManager().SetTimer(SemiFireTimerHandle, this,
			&AWeaponBase::ResetSemiFireCooldown, Interval, false);
	}
}

void AWeaponBase::ResetSemiFireCooldown()
{
	bFireCooldown = false;
}

void AWeaponBase::Fire()
{
	if (!CanFire())
	{
		GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
		return;
	}

	// bAmmoPerPellet=false(DMR)면 첫 번째 팰릿(CurrentPelletShot==0)일 때만 소비
	if (bAmmoPerPellet || CurrentPelletShot == 0)
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
		PC->ApplyRecoilShot();
	}

}

void AWeaponBase::HitscanFire()
{
	APawn* OwnerPawn       = Cast<APawn>(GetOwner());
	AController* OwnerCtrl = OwnerPawn ? OwnerPawn->GetController() : nullptr;

	FVector  CamLoc;
	FRotator CamRot;

	if (APlayerController* PC = Cast<APlayerController>(OwnerCtrl))
		PC->GetPlayerViewPoint(CamLoc, CamRot);
	else if (OwnerPawn)
	{
		CamLoc = OwnerPawn->GetActorLocation();
		CamRot = OwnerPawn->GetActorRotation();
	}
	else return;

	const bool    bIsPlayer  = Cast<APlayerController>(OwnerCtrl) != nullptr;
	const FVector TraceStart = bIsPlayer ? CamLoc + CamRot.Vector() * TraceStartOffset : CamLoc;
	const FVector BaseDir    = CamRot.Vector();

	float ActualDamage = Damage;
	if (CosmeticProjectileClass)
	{
		if (const AProjectileBase* CDO = CosmeticProjectileClass->GetDefaultObject<AProjectileBase>())
			ActualDamage = CDO->GetDamage();
	}

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	// 퍼짐 콘: PelletCount > 1이고 SpreadAngle > 0일 때만 계산
	const float SpreadTan  = (PelletCount > 1 && PelletSpreadAngle > 0.f)
		? FMath::Tan(FMath::DegreesToRadians(PelletSpreadAngle * 0.5f)) : 0.f;
	const FVector RightVec = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
	const FVector UpVec    = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Z);

	FHitResult RepresentativeHit; // 코스메틱/OnFire에 넘길 대표 히트

	for (int32 i = 0; i < FMath::Max(PelletCount, 1); ++i)
	{
		FVector PelletDir = BaseDir;
		if (SpreadTan > 0.f)
		{
			const float RandAngle  = FMath::FRandRange(0.f, 2.f * PI);
			const float RandRadius = FMath::FRandRange(0.f, SpreadTan);
			PelletDir = (BaseDir
				+ RightVec * FMath::Cos(RandAngle) * RandRadius
				+ UpVec    * FMath::Sin(RandAngle) * RandRadius).GetSafeNormal();
		}

		const FVector TraceEnd = TraceStart + PelletDir * Range;
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

		if (bDebugTrace)
			UKismetSystemLibrary::DrawDebugLine(this, TraceStart,
				Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd, FLinearColor::Red, 2.f, 1.f);

		if (Hit.bBlockingHit)
		{
			if (HitVFX)
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitVFX,
					Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

			if (Hit.GetActor())
				UGameplayStatics::ApplyPointDamage(Hit.GetActor(), ActualDamage,
					PelletDir, Hit, OwnerCtrl, this, nullptr);

			if (!RepresentativeHit.bBlockingHit)
				RepresentativeHit = Hit;
		}
	}

	if (bDebugTrace)
		UKismetSystemLibrary::DrawDebugSphere(this, TraceStart, 5.f, 8, FLinearColor::Green, 2.f);

	// 코스메틱 트레이서: 발사당 1개 (샷건 포함)
	if (CosmeticProjectileClass)
	{
		const FVector MuzzleLoc = WeaponMesh->DoesSocketExist(MuzzleSocketName)
			? WeaponMesh->GetSocketLocation(MuzzleSocketName)
			: GetActorLocation();
		const FVector TargetPoint = RepresentativeHit.bBlockingHit
			? RepresentativeHit.ImpactPoint : TraceStart + BaseDir * Range;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner      = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		AProjectileBase* Cosmetic = GetWorld()->SpawnActor<AProjectileBase>(
			CosmeticProjectileClass, MuzzleLoc, (TargetPoint - MuzzleLoc).Rotation(), SpawnParams);
		if (Cosmetic)
		{
			Cosmetic->SetActorEnableCollision(false);
			if (ProjectileSpeedOverride > 0.f)
				Cosmetic->OverrideSpeed(ProjectileSpeedOverride);
		}
	}

	OnFire(RepresentativeHit);
}

void AWeaponBase::ProjectileFire()
{
	if (!ProjectileClass) return;

	const FVector MuzzleLoc = WeaponMesh->DoesSocketExist(MuzzleSocketName)
		? WeaponMesh->GetSocketLocation(MuzzleSocketName)
		: GetActorLocation();

	APawn* OwnerPawn       = Cast<APawn>(GetOwner());
	AController* OwnerCtrl = OwnerPawn ? OwnerPawn->GetController() : nullptr;

	FVector  CamLoc;
	FRotator CamRot;

	if (APlayerController* PC = Cast<APlayerController>(OwnerCtrl))
		PC->GetPlayerViewPoint(CamLoc, CamRot);
	else if (OwnerPawn)
	{
		CamLoc = OwnerPawn->GetActorLocation();
		CamRot = OwnerPawn->GetActorRotation();
	}
	else return;

	const bool    bIsPlayerProj  = Cast<APlayerController>(OwnerCtrl) != nullptr;
	const FVector TraceStartProj = bIsPlayerProj ? CamLoc + CamRot.Vector() * TraceStartOffset : CamLoc;
	const FVector BaseDir        = CamRot.Vector();

	FVector BaseTarget = TraceStartProj + BaseDir * Range;
	if (bIsPlayerProj)
	{
		FHitResult CamHit;
		FCollisionQueryParams CamParams;
		CamParams.AddIgnoredActor(this);
		CamParams.AddIgnoredActor(GetOwner());
		if (GetWorld()->LineTraceSingleByChannel(CamHit, TraceStartProj, BaseTarget, ECC_Visibility, CamParams))
			BaseTarget = CamHit.ImpactPoint;
	}

	if (bDebugTrace)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, TraceStartProj, 5.f, 8, FLinearColor::Green, 2.f);
		UKismetSystemLibrary::DrawDebugLine(this, TraceStartProj, BaseTarget, FLinearColor::Red, 2.f, 1.f);
	}

	const float SpreadTan  = (PelletCount > 1 && PelletSpreadAngle > 0.f)
		? FMath::Tan(FMath::DegreesToRadians(PelletSpreadAngle * 0.5f)) : 0.f;
	const FVector RightVec = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
	const FVector UpVec    = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Z);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner      = GetOwner();
	SpawnParams.Instigator = OwnerPawn;

	for (int32 i = 0; i < FMath::Max(PelletCount, 1); ++i)
	{
		FVector PelletDir = (BaseTarget - MuzzleLoc).GetSafeNormal();
		if (SpreadTan > 0.f)
		{
			const float RandAngle  = FMath::FRandRange(0.f, 2.f * PI);
			const float RandRadius = FMath::FRandRange(0.f, SpreadTan);
			PelletDir = (BaseDir
				+ RightVec * FMath::Cos(RandAngle) * RandRadius
				+ UpVec    * FMath::Sin(RandAngle) * RandRadius).GetSafeNormal();
		}

		AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
			ProjectileClass, MuzzleLoc, PelletDir.Rotation(), SpawnParams);
		if (Projectile)
		{
			Projectile->AddIgnoredActor(this);
			Projectile->AddIgnoredActor(GetOwner());
			if (ProjectileSpeedOverride > 0.f)
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

void AWeaponBase::SetDropped(bool bDropped)
{
	bIsDropped = bDropped;
	if (bDropped)
	{
		// 드롭 직후 즉시 재픽업 방지: 0.5초 후 구체 활성화
		GetWorldTimerManager().SetTimer(PickupEnableTimerHandle, this, &AWeaponBase::EnablePickupSphere, 0.5f, false);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(PickupEnableTimerHandle);
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeaponBase::EnablePickupSphere()
{
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AlignWeaponToLeftHand();

	if (!bDebugGrip) return;

	const FVector RootLoc = GetActorLocation();

	// Actor root (WeaponSocket 어태치 기준점)
	DrawDebugSphere(GetWorld(), RootLoc, 3.f, 8, FColor::Green, false, -1.f, 0, 0.5f);
	DrawDebugString(GetWorld(), RootLoc + FVector(0, 0, 8), TEXT("Root"), nullptr, FColor::Green, 0.f, false);

	// LeftHandGripStart 소켓: 오른손 정렬 기준. 이 소켓이 Root에 맞춰져야 함.
	if (WeaponMesh->DoesSocketExist(LeftHandGripStartSocketName))
	{
		FTransform T = WeaponMesh->GetSocketTransform(LeftHandGripStartSocketName);
		DrawDebugSphere(GetWorld(), T.GetLocation(), 4.f, 8, FColor::Yellow, false, -1.f, 0, 0.5f);
		DrawDebugCoordinateSystem(GetWorld(), T.GetLocation(), T.GetRotation().Rotator(), 12.f, false, -1.f, 0, 0.8f);
		DrawDebugString(GetWorld(), T.GetLocation() + FVector(0, 0, 8),
			FString::Printf(TEXT("GripStart [%s]"), *LeftHandGripStartSocketName.ToString()),
			nullptr, FColor::Yellow, 0.f, false);
	}
	else
	{
		DrawDebugString(GetWorld(), RootLoc + FVector(0, 0, 16),
			FString::Printf(TEXT("NO SOCKET: %s"), *LeftHandGripStartSocketName.ToString()),
			nullptr, FColor::Red, 0.f, false);
	}

	// LeftHandGrip 소켓: ABP IK 왼손 위치 참조용.
	if (WeaponMesh->DoesSocketExist(LeftHandGripSocketName))
	{
		FTransform T = WeaponMesh->GetSocketTransform(LeftHandGripSocketName);
		DrawDebugSphere(GetWorld(), T.GetLocation(), 4.f, 8, FColor::Cyan, false, -1.f, 0, 0.5f);
		DrawDebugCoordinateSystem(GetWorld(), T.GetLocation(), T.GetRotation().Rotator(), 12.f, false, -1.f, 0, 0.8f);
		DrawDebugString(GetWorld(), T.GetLocation() + FVector(0, 0, 8),
			FString::Printf(TEXT("GripLeft [%s]"), *LeftHandGripSocketName.ToString()),
			nullptr, FColor::Cyan, 0.f, false);
	}
	else
	{
		DrawDebugString(GetWorld(), RootLoc + FVector(0, 0, 24),
			FString::Printf(TEXT("NO SOCKET: %s"), *LeftHandGripSocketName.ToString()),
			nullptr, FColor::Red, 0.f, false);
	}

	// bUseLeftHandGrip 상태
	DrawDebugString(GetWorld(), RootLoc + FVector(0, 0, 32),
		FString::Printf(TEXT("bUseLeftHandGrip: %s"), bUseLeftHandGrip ? TEXT("ON") : TEXT("OFF")),
		nullptr, bUseLeftHandGrip ? FColor::Green : FColor::Orange, 0.f, false);

	// 왼손 본 타겟 위치 (주황 구): 여기 없거나 이상한 위치면 본 이름 오류
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		const USkeletalMeshComponent* CharMesh = OwnerChar->GetMesh();
		if (CharMesh->DoesSocketExist(LeftHandSocketName))
		{
			const FVector LeftHandWorld = CharMesh->GetSocketLocation(LeftHandSocketName);
			DrawDebugSphere(GetWorld(), LeftHandWorld, 5.f, 8, FColor::Orange, false, -1.f, 0, 0.8f);
			DrawDebugLine(GetWorld(), RootLoc, LeftHandWorld, FColor::Orange, false, -1.f, 0, 0.5f);
			DrawDebugString(GetWorld(), LeftHandWorld + FVector(0, 0, 8),
				FString::Printf(TEXT("LeftHandSocket [%s]"), *LeftHandSocketName.ToString()),
				nullptr, FColor::Orange, 0.f, false);
		}
		else
		{
			DrawDebugString(GetWorld(), RootLoc + FVector(0, 0, 40),
				FString::Printf(TEXT("SOCKET NOT FOUND: %s"), *LeftHandSocketName.ToString()),
				nullptr, FColor::Red, 0.f, false);
		}
	}
}

void AWeaponBase::RefreshMeshTransform()
{
	WeaponMesh->SetRelativeScale3D(WeaponMeshScale);

	if (bUseLeftHandGrip && WeaponMesh->DoesSocketExist(GripSocketName))
	{
		// GripSocket이 WeaponSocket(actor root)에 정렬되도록 역행렬 적용.
		// LeftHandGripStart·LeftHandGrip은 방향 정의 및 왼손 IK 참조용으로 별도 사용.
		WeaponMesh->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
		FTransform GripTF = WeaponMesh->GetSocketTransform(GripSocketName, RTS_Actor);
		GripTF.SetScale3D(FVector::OneVector);
		FTransform Correction = GripTF.Inverse();
		Correction.SetLocation(Correction.GetLocation() + WeaponMeshLocationOffset);
		WeaponMesh->SetRelativeLocationAndRotation(Correction.GetLocation(), Correction.GetRotation());
	}
	else
	{
		WeaponMesh->SetRelativeLocationAndRotation(WeaponMeshLocationOffset, WeaponMeshRotationOffset);
	}

	BaseRelativeTransform = WeaponMesh->GetRelativeTransform();
}

void AWeaponBase::AlignWeaponToLeftHand()
{
	if (!bUseLeftHandGrip) return;
	if (!WeaponMesh->DoesSocketExist(LeftHandGripSocketName)) return;

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;

	const USkeletalMeshComponent* CharMesh = OwnerChar->GetMesh();
	if (!CharMesh->DoesSocketExist(LeftHandSocketName)) return;

	// 매 틱 베이스 트랜스폼에서 시작해 드리프트 방지
	WeaponMesh->SetRelativeTransform(BaseRelativeTransform);

	// 본 대신 소켓 참조 → 적 포함 모든 캐릭터에서 공통 동작
	const FTransform ActorTF    = GetActorTransform();
	const FVector LeftHandLocal = ActorTF.InverseTransformPosition(CharMesh->GetSocketLocation(LeftHandSocketName));
	const FVector GripLeftLocal = ActorTF.InverseTransformPosition(WeaponMesh->GetSocketLocation(LeftHandGripSocketName));

	if (GripLeftLocal.IsNearlyZero() || LeftHandLocal.IsNearlyZero()) return;

	const FVector GripDir   = GripLeftLocal.GetSafeNormal();
	const FVector TargetDir = LeftHandLocal.GetSafeNormal();

	// 두 방향이 반대일 때 FindBetweenNormals는 임의 축으로 뒤집힘 → 무기 Up을 힌트 축으로 사용
	FQuat DeltaLocal;
	if (FVector::DotProduct(GripDir, TargetDir) < -0.9999f)
	{
		const FVector HintAxis = BaseRelativeTransform.GetRotation().RotateVector(FVector::UpVector);
		DeltaLocal = FQuat(HintAxis.GetSafeNormal(), PI);
	}
	else
	{
		DeltaLocal = FQuat::FindBetweenNormals(GripDir, TargetDir);
	}

	if (DeltaLocal.IsIdentity(0.001f)) return;

	// BaseRelativeTransform 기준으로 직접 계산 (SetRelativeTransform 후 중복 Set 방지)
	WeaponMesh->SetRelativeLocationAndRotation(
		DeltaLocal.RotateVector(BaseRelativeTransform.GetLocation()),
		DeltaLocal * BaseRelativeTransform.GetRotation());
}

FTransform AWeaponBase::GetLeftHandGripTransform() const
{
	if (WeaponMesh->DoesSocketExist(LeftHandGripSocketName))
		return WeaponMesh->GetSocketTransform(LeftHandGripSocketName);
	return WeaponMesh->GetComponentTransform();
}

USoundBase* AWeaponBase::GetPickupSound() const
{
	return WeaponData ? WeaponData->PickupSound : nullptr;
}

FText AWeaponBase::GetWeaponDisplayName() const
{
	return WeaponData ? WeaponData->WeaponDisplayName : FText::GetEmpty();
}

FVector AWeaponBase::GetMuzzleLocation() const
{
	if (WeaponMesh->DoesSocketExist(MuzzleSocketName))
		return WeaponMesh->GetSocketLocation(MuzzleSocketName);
	return GetActorLocation();
}

void AWeaponBase::OnPickupSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsDropped) return;
	if (APlayerCharacter* PC = Cast<APlayerCharacter>(OtherActor))
	{
		PC->OnWeaponPickupRangeEnter(this);
	}
}

void AWeaponBase::OnPickupSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APlayerCharacter* PC = Cast<APlayerCharacter>(OtherActor))
	{
		PC->OnWeaponPickupRangeExit(this);
	}
}
