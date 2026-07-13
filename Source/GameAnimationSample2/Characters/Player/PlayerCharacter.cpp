// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "WeaponBase.h"
#include "GrenadeBase.h"
#include "TraversalComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MotionWarpingComponent.h"
#include "Components/ArrowComponent.h"
#include "HUDDataAsset.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Spring Arm
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength        = 300.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	// Camera
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	// Movement defaults
	WalkSpeed     = 400.0f;
	SprintSpeed   = 700.0f;
	AimWalkSpeed  = 200.0f;
	NormalFOV     = 90.0f;
	AimFOV        = 60.0f;
	bIsAiming     = false;

	AllowedActions = 0xFF;

	bUseControllerRotationYaw                              = false;
	GetCharacterMovement()->bOrientRotationToMovement      = true;
	GetCharacterMovement()->MaxWalkSpeed                   = WalkSpeed;

	// Traversal
	TraversalComponent    = CreateDefaultSubobject<UTraversalComponent>(TEXT("TraversalComponent"));
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	// AimArrow
	AimArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("AimArrow"));
	AimArrow->SetupAttachment(GetMesh());
	AimArrow->SetArrowColor(FColor::Cyan);

	// Trajectory Spline
	TrajectorySpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrajectorySpline"));
	TrajectorySpline->SetupAttachment(RootComponent);

	// Weapon
	CurrentWeapon      = nullptr;
	CurrentWeaponIndex = -1;
	MaxWeaponSlots     = 3;
	WeaponAttachSocket = TEXT("WeaponSocketRight");	// 미카 오른손 본 소켓 (메시에 실제 존재). "WeaponSocket"은 없는 소켓이라 루트로 폴백됐었음.
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CameraComponent->FieldOfView         = NormalFOV;
	GetCharacterMovement()->MaxWalkSpeed         = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchWalkSpeed;

	SpringArmComponent->SocketOffset.Y = DefaultSocketOffsetY;
	NormalSocketOffsetY                 = DefaultSocketOffsetY;

	GetCharacterMovement()->NavAgentProps.bCanCrouch      = true;
	GetCharacterMovement()->bCrouchMaintainsBaseLocation  = true;

	// Trajectory mesh pool
	for (int32 i = 0; i < MaxTrajectorySegments; ++i)
	{
		USplineMeshComponent* SMC = NewObject<USplineMeshComponent>(this);
		SMC->SetMobility(EComponentMobility::Movable);
		SMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SMC->SetVisibility(false);
		SMC->RegisterComponent();
		SplineMeshPool.Add(SMC);
	}

	// Spawn default weapons
	for (int32 i = 0; i < DefaultWeaponClasses.Num() && i < MaxWeaponSlots; ++i)
	{
		if (!DefaultWeaponClasses[i]) continue;
		FActorSpawnParameters Params;
		Params.Owner = this;
		AWeaponBase* Weapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClasses[i], FVector::ZeroVector, FRotator::ZeroRotator, Params);
		PickupWeapon(Weapon);
	}
}

void APlayerCharacter::Jump()
{
	if (TraversalComponent && TraversalComponent->TryTraversal()) return;
	Super::Jump();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC) return;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (IMC_Default)
			{
				Subsystem->AddMappingContext(IMC_Default, 0);
			}
		}
	}

	auto Bind = [&](const FString& Name, ETriggerEvent Event, auto Func)
	{
		if (const UInputAction* IA = FindActionInIMC(Name))
			EIC->BindAction(IA, Event, this, Func);
	};

	Bind(TEXT("Move"),    ETriggerEvent::Triggered, &APlayerCharacter::HandleMove);
	Bind(TEXT("Look"),    ETriggerEvent::Triggered, &APlayerCharacter::HandleLook);
	Bind(TEXT("Jump"),    ETriggerEvent::Started,   &APlayerCharacter::Jump);
	Bind(TEXT("Jump"),    ETriggerEvent::Completed, &APlayerCharacter::StopJumping);
	Bind(TEXT("Run"),     ETriggerEvent::Started,   &APlayerCharacter::StartSprint);
	Bind(TEXT("Run"),     ETriggerEvent::Completed, &APlayerCharacter::StopSprint);
	Bind(TEXT("Aim"),     ETriggerEvent::Started,   &APlayerCharacter::StartAim);
	Bind(TEXT("Aim"),     ETriggerEvent::Completed, &APlayerCharacter::StopAim);
	Bind(TEXT("Shoot"),   ETriggerEvent::Started,   &APlayerCharacter::StartFire);
	Bind(TEXT("Shoot"),   ETriggerEvent::Completed, &APlayerCharacter::StopFire);
	Bind(TEXT("Crouch"),  ETriggerEvent::Started,   &APlayerCharacter::ToggleCrouch);
	Bind(TEXT("Reload"),  ETriggerEvent::Started,   &APlayerCharacter::Reload);
	Bind(TEXT("Weapon1"), ETriggerEvent::Started,   &APlayerCharacter::EquipWeaponSlot1);
	Bind(TEXT("Weapon2"), ETriggerEvent::Started,   &APlayerCharacter::EquipWeaponSlot2);
	Bind(TEXT("Weapon3"), ETriggerEvent::Started,   &APlayerCharacter::EquipWeaponSlot3);
	Bind(TEXT("Swap"),    ETriggerEvent::Started,   &APlayerCharacter::SwapToLastWeapon);
	Bind(TEXT("Throw"),    ETriggerEvent::Started,   &APlayerCharacter::StartGrenadeThrow);
	Bind(TEXT("Throw"),    ETriggerEvent::Completed, &APlayerCharacter::ReleaseGrenadeThrow);
	Bind(TEXT("Interact"), ETriggerEvent::Started,   &APlayerCharacter::TryPickupNearbyWeapon);
}

const UInputAction* APlayerCharacter::FindActionInIMC(const FString& NameContains) const
{
	if (!IMC_Default) return nullptr;
	for (const FEnhancedActionKeyMapping& Mapping : IMC_Default->GetMappings())
	{
		if (Mapping.Action && Mapping.Action->GetName().Contains(NameContains))
		{
			return Mapping.Action.Get();
		}
	}
	return nullptr;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsPreparingThrow)
	{
		UpdateTrajectory();
	}
	else if (bIsAiming && CurrentWeapon && CurrentWeapon->ShowsArcTrajectory())
	{
		// 유탄 발사기: 발사 중이 아니라 조준하는 동안 계속 궤도 표시
		UpdateWeaponTrajectory();
	}
	else
	{
		ClearTrajectory();
	}
	UpdateCoverPeek(DeltaTime);
	UpdateCrouchCamera(DeltaTime);

	if (bIsAiming)
	{
		UpdateAimTurn(DeltaTime);
	}

	UpdateAimSpinePitch(DeltaTime);
	UpdateCrosshairSpread(DeltaTime);
	UpdateRecoil(DeltaTime);

	bIsFalling = GetCharacterMovement()->IsFalling();
	if (bIsFalling)
	{
		const float ZVel = GetVelocity().Z;
		CurrentFallSpeed = (ZVel < 0.f) ? -ZVel : 0.f;
	}
	else
	{
		CurrentFallSpeed = 0.f;
	}
}

void APlayerCharacter::UpdateCoverPeek(float DeltaTime)
{
	float TargetY = NormalSocketOffsetY;

	if (bIsAiming)
	{
		FVector Origin = GetActorLocation();
		FVector Right  = FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::Y);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		FHitResult LeftHit, RightHit;
		bool bLeftCover  = GetWorld()->LineTraceSingleByChannel(LeftHit,  Origin, Origin - Right * CoverTraceDistance, ECC_WorldStatic, Params);
		bool bRightCover = GetWorld()->LineTraceSingleByChannel(RightHit, Origin, Origin + Right * CoverTraceDistance, ECC_WorldStatic, Params);

		if (bLeftCover && !bRightCover)
			TargetY = NormalSocketOffsetY + CoverPeekOffset;
		else if (bRightCover && !bLeftCover)
			TargetY = NormalSocketOffsetY - CoverPeekOffset;
	}

	SpringArmComponent->SocketOffset.Y = FMath::FInterpTo(
		SpringArmComponent->SocketOffset.Y, TargetY, DeltaTime, CoverPeekInterpSpeed);
}

void APlayerCharacter::UpdateAimTurn(float DeltaTime)
{
	const float ControlYaw = GetControlRotation().Yaw;
	const float ActorYaw   = GetActorRotation().Yaw;
	AimYaw = FMath::UnwindDegrees(ControlYaw - ActorYaw);

	if (GetVelocity().SizeSquared2D() > 1.f)
	{
		SetActorRotation(FRotator(0.f, ControlYaw, 0.f));
		AimYaw          = 0.f;
		bIsTurningRight = false;
		bIsTurningLeft  = false;
		return;
	}

	if (bIsTurningRight || bIsTurningLeft)
	{
		const float Delta     = FMath::UnwindDegrees(ControlYaw - GetActorRotation().Yaw);
		const float RotStep   = TurnRotationSpeed * DeltaTime;
		const float RotAmount = FMath::Sign(Delta) * FMath::Min(FMath::Abs(Delta), RotStep);

		FRotator NewRot = GetActorRotation();
		NewRot.Yaw += RotAmount;
		SetActorRotation(NewRot);

		AimYaw = FMath::UnwindDegrees(ControlYaw - NewRot.Yaw);

		if (FMath::Abs(AimYaw) < 5.f)
		{
			bIsTurningRight = false;
			bIsTurningLeft  = false;
		}
		return;
	}

	if (AimYaw >  TurnRightThreshold) { bIsTurningRight = true; return; }
	if (AimYaw < -TurnLeftThreshold)  { bIsTurningLeft  = true; return; }

	if (SoftTurnSpeed > 0.f && FMath::Abs(AimYaw) > 0.1f)
	{
		const float RotStep   = SoftTurnSpeed * DeltaTime;
		const float RotAmount = FMath::Sign(AimYaw) * FMath::Min(FMath::Abs(AimYaw), RotStep);

		FRotator NewRot = GetActorRotation();
		NewRot.Yaw += RotAmount;
		SetActorRotation(NewRot);

		AimYaw = FMath::UnwindDegrees(ControlYaw - NewRot.Yaw);
	}
}

void APlayerCharacter::UpdateCrosshairSpread(float DeltaTime)
{
	if (!HUDData) return;

	float BaseSpread = 0.f;
	const float Speed = GetVelocity().Size2D();

	if (Speed > SprintSpeed * 0.8f)
		BaseSpread = HUDData->SpreadSprinting;
	else if (Speed > 10.f)
		BaseSpread = HUDData->SpreadWalking;

	if (bIsAiming)
		BaseSpread = FMath::Max(0.f, BaseSpread + HUDData->SpreadAimingDelta);

	SpreadAdditive = FMath::Max(SpreadAdditive - HUDData->SpreadRecoverySpeed * DeltaTime, 0.f);
	CurrentCrosshairSpread = BaseSpread + SpreadAdditive;
}

void APlayerCharacter::AddCrosshairSpread(float Amount)
{
	SpreadAdditive += Amount;
}

void APlayerCharacter::UpdateAimSpinePitch(float DeltaTime)
{
	float TargetPitch = 0.f;

	if (bIsTurningRight || bIsTurningLeft)
	{
		AimSpinePitch = FMath::FInterpTo(AimSpinePitch, 0.f, DeltaTime, AimSpineInterpSpeed);
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && AimArrow)
	{
		FVector CamLoc; FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);

		FVector TraceEnd = CamLoc + CamRot.Vector() * 10000.f;
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		FVector AimTarget = GetWorld()->LineTraceSingleByChannel(
			Hit, CamLoc, TraceEnd, ECC_Visibility, Params)
			? Hit.ImpactPoint : TraceEnd;

		FVector ToTarget = (AimTarget - AimArrow->GetComponentLocation()).GetSafeNormal();
		float RawPitch = FMath::RadiansToDegrees(FMath::Asin(ToTarget.Z));
		TargetPitch = FMath::ClampAngle(RawPitch, -AimSpinePitchClamp, AimSpinePitchClamp);
	}

	AimSpinePitch = FMath::FInterpTo(AimSpinePitch, TargetPitch, DeltaTime, AimSpineInterpSpeed);
}

void APlayerCharacter::HandleMove(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Axis.Y);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Axis.X);
	}
}

void APlayerCharacter::HandleLook(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

bool APlayerCharacter::CanDo(ECharacterAction Action) const
{
	return (AllowedActions & static_cast<int32>(Action)) != 0;
}

void APlayerCharacter::Allow(ECharacterAction Action)
{
	AllowedActions |= static_cast<int32>(Action);
}

void APlayerCharacter::Block(ECharacterAction Action)
{
	AllowedActions &= ~static_cast<int32>(Action);
}

void APlayerCharacter::StartSprint()
{
	if (!CanDo(ECharacterAction::Sprint)) return;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void APlayerCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : WalkSpeed;
}

void APlayerCharacter::StartAim()
{
	if (!CanDo(ECharacterAction::Aim)) return;
	bIsAiming                                         = true;
	CameraComponent->FieldOfView                      = AimFOV;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed              = AimWalkSpeed;
	Block(ECharacterAction::Sprint);
	StopSprint();
}

void APlayerCharacter::StopAim()
{
	bIsAiming                                         = false;
	CameraComponent->FieldOfView                      = NormalFOV;
	bUseControllerRotationYaw                         = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed              = WalkSpeed;
	Allow(ECharacterAction::Sprint);
	bIsTurningRight = false;
	bIsTurningLeft  = false;
	AimYaw          = 0.f;
}

void APlayerCharacter::ToggleCrouch()
{
	// bIsCrouched는 엔진이 Crouch/UnCrouch로 세팅하는 단일 진실 공급원
	if (bIsCrouched)
		UnCrouch();
	else
		Crouch();
}

// 앉기 카메라 보정 — 스프링암 TargetOffset.Z(월드 상하)만 사용.
// 즉시 전환(bSmoothCrouchCamera=false)은 OnStartCrouch/OnEndCrouch에서 처리하고,
// 보간 모드는 이 Tick 함수가 bIsCrouched 상태로 목표값을 향해 부드럽게 이동시킨다.
void APlayerCharacter::UpdateCrouchCamera(float DeltaTime)
{
	if (!bSmoothCrouchCamera || !SpringArmComponent) return;

	const float TargetZ = bIsCrouched ? CrouchCameraZOffset : 0.f;
	SpringArmComponent->TargetOffset.Z = FMath::FInterpTo(
		SpringArmComponent->TargetOffset.Z, TargetZ, DeltaTime, CrouchCameraInterpSpeed);
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (!bSmoothCrouchCamera && SpringArmComponent)
		SpringArmComponent->TargetOffset.Z = CrouchCameraZOffset;
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (!bSmoothCrouchCamera && SpringArmComponent)
		SpringArmComponent->TargetOffset.Z = 0.f;
}

void APlayerCharacter::SetGravityDirection(FVector NewDirection)
{
	if (NewDirection.IsNearlyZero()) return;
	GetCharacterMovement()->SetGravityDirection(NewDirection.GetSafeNormal());
}

void APlayerCharacter::ResetGravity()
{
	GetCharacterMovement()->SetGravityDirection(FVector(0.f, 0.f, -1.f));
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	const float LandingSpeed = CurrentFallSpeed;
	const bool  bHard        = LandingSpeed >= HardLandingSpeedThreshold;

	CurrentFallSpeed = 0.f;
	bIsFalling       = false;
	bIsHardLanding   = bHard;

	OnLanding(bHard);

	Super::Landed(Hit);
}

void APlayerCharacter::OnLanding_Implementation(bool bHardLanding)
{
}

void APlayerCharacter::ResetHardLanding()
{
	bIsHardLanding = false;
}
