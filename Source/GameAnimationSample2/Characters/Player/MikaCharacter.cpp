// Copyright Epic Games, Inc. All Rights Reserved.

#include "MikaCharacter.h"
#include "IDestructible.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AMikaCharacter::AMikaCharacter()
{
	PunchHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("PunchHitbox"));
	PunchHitbox->SetupAttachment(GetMesh(), TEXT("WeaponSocketRight"));	// "hand_r"은 ValveBiped 스켈레톤에 없는 소켓이라 루트로 폴백됐었음. 오른손 소켓 사용.
	PunchHitbox->SetBoxExtent(FVector(20.f, 20.f, 20.f));
	PunchHitbox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	PunchHitbox->SetGenerateOverlapEvents(false);
}

void AMikaCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (MikaData)
	{
		// Stats
		MaxHealth                  = MikaData->MaxHealth;
		// Movement
		WalkSpeed                  = MikaData->WalkSpeed;
		SprintSpeed                = MikaData->SprintSpeed;
		AimWalkSpeed               = MikaData->AimWalkSpeed;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		// Camera
		NormalFOV                  = MikaData->NormalFOV;
		AimFOV                     = MikaData->AimFOV;
		CoverTraceDistance         = MikaData->CoverTraceDistance;
		CoverPeekOffset            = MikaData->CoverPeekOffset;
		CoverPeekInterpSpeed       = MikaData->CoverPeekInterpSpeed;
		DefaultSocketOffsetY       = MikaData->DefaultSocketOffsetY;
			AimSpinePitchClamp         = MikaData->AimSpinePitchClamp;
			AimSpineInterpSpeed        = MikaData->AimSpineInterpSpeed;
			// TurnInPlace
		TurnRightThreshold         = MikaData->TurnRightThreshold;
		TurnLeftThreshold          = MikaData->TurnLeftThreshold;
		TurnRotationSpeed          = MikaData->TurnRotationSpeed;
		SoftTurnSpeed              = MikaData->SoftTurnSpeed;
		// Fall
		HardLandingSpeedThreshold  = MikaData->HardLandingSpeedThreshold;
		// Grenade / Weapon
		GrenadeThrowSpeed          = MikaData->GrenadeThrowSpeed;
			GrenadeCount               = MikaData->MaxGrenadeCount;
		WeaponSwapDelay            = MikaData->WeaponSwapDelay;
		CurrentHealth              = MaxHealth;
		// Punch
		PunchDamage                = MikaData->PunchDamage;
		PunchCooldown              = MikaData->PunchCooldown;
		ChargeGravityScale         = MikaData->ChargeGravityScale;
		ChargeMovementScale        = MikaData->ChargeMovementScale;
		ChargeBrakingDeceleration  = MikaData->ChargeBrakingDeceleration;
		ChargeAirBrakingDeceleration = MikaData->ChargeAirBrakingDeceleration;
		MinChargeTime              = MikaData->MinChargeTime;
		MaxChargeTime              = MikaData->MaxChargeTime;
		ForcedMaxChargeTime        = MikaData->ForcedMaxChargeTime;
		MinDashSpeed               = MikaData->MinDashSpeed;
		MaxDashSpeed               = MikaData->MaxDashSpeed;
		DashDuration               = MikaData->DashDuration;
		DashBrakingDeceleration    = MikaData->DashBrakingDeceleration;
		// Punch Camera
		ChargeSpringArmLength      = MikaData->ChargeSpringArmLength;
		ChargeFOV                  = MikaData->ChargeFOV;
		DashSpringArmLength        = MikaData->DashSpringArmLength;
		DashFOV                    = MikaData->DashFOV;
		CameraInterpSpeed          = MikaData->CameraInterpSpeed;
		// Landing
		LandingDamage              = MikaData->LandingDamage;
		LandingRadius              = MikaData->LandingRadius;
		LandingDownAngle           = MikaData->LandingDownAngle;
		LandingMinHeight           = MikaData->LandingMinHeight;
		LandingDiveSpeed           = MikaData->LandingDiveSpeed;
	}

	PunchHitbox->OnComponentBeginOverlap.AddDynamic(this, &AMikaCharacter::OnPunchHitboxOverlap);
	NormalSpringArmLength        = SpringArmComponent->TargetArmLength;
	DefaultBrakingDeceleration     = GetCharacterMovement()->BrakingDecelerationWalking;
	DefaultBrakingDecelerationFly  = GetCharacterMovement()->BrakingDecelerationFlying;
	DefaultBrakingDecelerationFall = GetCharacterMovement()->BrakingDecelerationFalling;
}

void AMikaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetArm, TargetFov;
	if (bIsAiming)
	{
		TargetArm = NormalSpringArmLength;
		TargetFov = AimFOV;
	}
	else if (bIsDashing)
	{
		TargetArm = DashSpringArmLength;
		TargetFov = DashFOV;
	}
	else if (bIsChargingPunch)
	{
		TargetArm = ChargeSpringArmLength;
		TargetFov = ChargeFOV;
	}
	else
	{
		TargetArm = NormalSpringArmLength;
		TargetFov = NormalFOV;
	}

	SpringArmComponent->TargetArmLength = FMath::FInterpTo(
		SpringArmComponent->TargetArmLength, TargetArm, DeltaTime, CameraInterpSpeed);
	CameraComponent->FieldOfView = FMath::FInterpTo(
		CameraComponent->FieldOfView, TargetFov, DeltaTime, CameraInterpSpeed);

	// 충전 중 캐릭터가 카메라 yaw 방향을 바라봄
	if (bIsChargingPunch)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FRotator CtrlRot = PC->GetControlRotation();
			SetActorRotation(FRotator(0.f, CtrlRot.Yaw, 0.f));
		}
	}
}

// --- StartFire / StopFire 오버라이드 ---

void AMikaCharacter::StartFire()
{
	if (bIsAiming)
	{
		Super::StartFire();
		return;
	}
	if (!bCanPunch) return;

	bIsChargingPunch = true;
	ChargeStartTime  = GetWorld()->GetTimeSeconds();
	GetCharacterMovement()->GravityScale                 = ChargeGravityScale;
	GetCharacterMovement()->MaxWalkSpeed                 = WalkSpeed * ChargeMovementScale;
	GetCharacterMovement()->BrakingDecelerationWalking   = ChargeBrakingDeceleration;
	GetCharacterMovement()->BrakingDecelerationFalling   = ChargeAirBrakingDeceleration;
	GetWorldTimerManager().SetTimer(AutoReleaseTimerHandle, this, &AMikaCharacter::StopFire, ForcedMaxChargeTime, false);
}

void AMikaCharacter::StopFire()
{
	if (bIsAiming && !bIsChargingPunch)
	{
		Super::StopFire();
		return;
	}
	if (!bIsChargingPunch) return;

	bIsChargingPunch = false;
	GetWorldTimerManager().ClearTimer(AutoReleaseTimerHandle);
	GetCharacterMovement()->GravityScale                 = 1.f;
	GetCharacterMovement()->MaxWalkSpeed                 = WalkSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking   = DefaultBrakingDeceleration;
	GetCharacterMovement()->BrakingDecelerationFalling   = DefaultBrakingDecelerationFall;

	float HeldTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	if (HeldTime < MinChargeTime) return;

	// 랜딩 조건 충족 시 다이브
	if (CanTriggerLanding())
	{
		bCanPunch        = false;
		bIsDivingLanding = true;
		LaunchCharacter(FVector(0.f, 0.f, -LandingDiveSpeed), false, true);
		return;
	}

	float ChargeRatio = FMath::Clamp((HeldTime - MinChargeTime) / (MaxChargeTime - MinChargeTime), 0.f, 1.f);
	StartDash(ChargeRatio);
}

// --- 쿨타임 ---

void AMikaCharacter::StartPunchCooldown()
{
	bCanPunch = false;
	GetWorldTimerManager().SetTimer(PunchCooldownTimerHandle, this, &AMikaCharacter::ResetPunchCooldown, PunchCooldown, false);
}

void AMikaCharacter::ResetPunchCooldown()
{
	bCanPunch = true;
}

// --- 랜딩 조건 ---

bool AMikaCharacter::CanTriggerLanding() const
{
	if (!bIsAiming) return false;

	const float Pitch = FMath::UnwindDegrees(GetControlRotation().Pitch);
	if (Pitch > -LandingDownAngle) return false;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	FVector Start = GetActorLocation();
	FVector End   = Start - FVector(0.f, 0.f, LandingMinHeight);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params)) return false;

	return true;
}

void AMikaCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (!bIsDivingLanding) return;
	bIsDivingLanding = false;
	MikaLanding();
}

// --- 대시 ---

void AMikaCharacter::StartDash(float ChargeRatio)
{
	HitActorsDuringDash.Empty();
	bIsDashing = true;
	bCanPunch  = false;

	APlayerController* PC = Cast<APlayerController>(GetController());
	FVector DashDir = GetActorForwardVector();
	if (PC)
	{
		FVector Loc; FRotator Rot;
		PC->GetPlayerViewPoint(Loc, Rot);
		DashDir = Rot.Vector();
		DashDir.Normalize();
	}

	FRotator FaceRot(0.f, DashDir.Rotation().Yaw, 0.f);
	SetActorRotation(FaceRot);
	bUseControllerRotationYaw                         = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	float Speed = FMath::Lerp(MinDashSpeed, MaxDashSpeed, ChargeRatio);
	GetCharacterMovement()->BrakingFrictionFactor    = 0.f;
	GetCharacterMovement()->BrakingDecelerationFlying = DashBrakingDeceleration;
	LaunchCharacter(DashDir * Speed, true, true);

	PunchHitbox->SetGenerateOverlapEvents(true);

	GetWorldTimerManager().SetTimer(DashEndTimerHandle, this, &AMikaCharacter::EndDash, DashDuration, false);
}

void AMikaCharacter::EndDash()
{
	bIsDashing = false;
	PunchHitbox->SetGenerateOverlapEvents(false);
	HitActorsDuringDash.Empty();
	GetCharacterMovement()->BrakingFrictionFactor    = 2.f;
	GetCharacterMovement()->BrakingDecelerationFlying = DefaultBrakingDecelerationFly;

	if (!bIsAiming)
	{
		bUseControllerRotationYaw                         = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	StartPunchCooldown();
}

// --- 히트박스 오버랩 ---

void AMikaCharacter::OnPunchHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;
	if (HitActorsDuringDash.Contains(OtherActor)) return;

	HitActorsDuringDash.Add(OtherActor);

	// IDestructibleObject이면 즉각 파괴, 아니면 일반 데미지
	if (IDestructibleObject* Destructible = Cast<IDestructibleObject>(OtherActor))
	{
		Destructible->Execute_DestroyByPunch(OtherActor, this);
	}
	else
	{
		UGameplayStatics::ApplyDamage(OtherActor, PunchDamage, GetController(), this, UDamageType::StaticClass());
	}
}

// --- 미카 랜딩 ---

void AMikaCharacter::MikaLanding()
{
	TArray<AActor*> Ignored;
	Ignored.Add(this);
	UGameplayStatics::ApplyRadialDamage(
		GetWorld(), LandingDamage, GetActorLocation(), LandingRadius,
		nullptr, Ignored, this, GetController(), true, ECC_Visibility);

	StartPunchCooldown();
}
