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
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MotionWarpingComponent.h"
#include "Components/ArrowComponent.h"
#include "Animation/AnimMontage.h"
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
	// ABP가 이번 프레임 Tick에서 계산한 값(LandPoseAlpha 등)을 읽도록 메시를 캐릭터 Tick 뒤에 돌린다
	GetMesh()->PrimaryComponentTick.AddPrerequisite(this, PrimaryActorTick);
	CameraComponent->FieldOfView         = NormalFOV;
	GetCharacterMovement()->MaxWalkSpeed         = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchWalkSpeed;

	ApplyDefaultSocketOffset();
	NormalSocketOffsetZ = SpringArmComponent->SocketOffset.Z;
	// 평상시(서 있는 포즈) 골반과 캡슐 중심의 높이 차 — 공중에서 이 값을 넘는 만큼만 카메라를 올린다
	if (GetMesh())
		CameraFollowBaseGap = GetMesh()->GetSocketLocation(CameraFollowMeshBone).Z - GetActorLocation().Z;

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
	Bind(TEXT("Move"),    ETriggerEvent::Completed, &APlayerCharacter::HandleMoveEnd);
	Bind(TEXT("Dodge"),   ETriggerEvent::Started,   &APlayerCharacter::StartDodge);
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
	UpdateWeaponDebugTrail();
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
	// 공중에서는 지상 로코모션 속도를 0으로 — 달리다 점프해도 공중 모션이 그대로 나온다
	const float HorizontalSpeed = GetVelocity().Size2D();
	LocoGroundSpeed = (bFreezeLocoSpeedInAir && bIsFalling) ? 0.f : HorizontalSpeed;

	// 착지 직후에도 잠시 공중 포즈를 유지해 착지 모션이 끝까지 재생되게 한다 (점프 모션을 느리게 틀면 그만큼 길게).
	// 단, 플레이어가 이동·행동을 시작하면 그 자리에서 끊는다 — 안 그러면 착지 포즈에 갇힌다
	const bool bInLandHold = GetWorld()->TimeSince(LandedTime) < GetLandHoldTime() && !IsLandPoseInterrupted();
	bIsInAirPose = bIsFalling || bInLandHold;
	AirPoseAlpha = FMath::FInterpTo(AirPoseAlpha, bIsInAirPose ? 1.f : 0.f, DeltaTime, AirPoseBlendSpeed);
	UpdateLandingPrediction();

	// 착지 모션은 전신 — 착지 직전 예측부터 착지 후 유지 시간까지
	const bool bLandPose = bIsLandingSoon || (!bIsFalling && bInLandHold);
	// 켜질 땐 즉시(예측을 놓쳐도 착지 프레임에 바로 전신), 꺼질 땐 부드럽게
	LandPoseAlpha = bLandPose ? 1.f : FMath::FInterpTo(LandPoseAlpha, 0.f, DeltaTime, AirPoseBlendSpeed);
}

bool APlayerCharacter::IsLandPoseInterrupted() const
{
	// 이동 입력이 들어왔거나 조준을 시작하면 착지 모션을 기다리지 않는다
	return bIsAiming || !GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero();
}

float APlayerCharacter::GetLandHoldTime() const
{
	// 클립을 지정해두면 길이가 기준 — 클립을 잘라내거나 늘려도 자동으로 따라간다
	const float Rate = FMath::Max(LandAnimPlayRate, 0.1f);   // 착지 유지 시간은 착지 재생 속도 기준
	if (LandAnimation)
	{
		const float Remaining = LandAnimation->GetPlayLength() - LandAnimStartTime;
		return FMath::Max(Remaining, 0.f) / Rate;
	}
	return LandPoseHoldTime / Rate;
}

void APlayerCharacter::UpdateLandingPrediction()
{
	bIsLandingSoon = false;
	if (!bIsFalling || LandAnticipationTime <= 0.f) return;

	const float FallSpeed = CurrentFallSpeed;   // 하강 중에만 양수
	if (FallSpeed <= KINDA_SMALL_NUMBER) return;

	// 이 시간 안에 닿을 거리만 확인하면 된다 (캡슐 그대로 훑어 모서리·경사도 실제 착지와 같게)
	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	// 낙하 중 계속 가속하므로 중력까지 포함: v·t + ½·g·t²
	const float Gravity = -GetCharacterMovement()->GetGravityZ();
	const float CheckDistance = FallSpeed * LandAnticipationTime
		+ 0.5f * Gravity * LandAnticipationTime * LandAnticipationTime;
	const FVector Start = GetActorLocation();
	const FVector End   = Start - FVector(0.f, 0.f, CheckDistance);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(LandingPrediction), false, this);
	FCollisionResponseParams Response;
	Capsule->InitSweepCollisionParams(Params, Response);
	const FCollisionShape Shape = FCollisionShape::MakeCapsule(
		Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight());

	if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity,
		Capsule->GetCollisionObjectType(), Shape, Params, Response))
	{
		// 걸을 수 있는 바닥만 착지로 친다 (벽을 스치는 건 제외)
		bIsLandingSoon = GetCharacterMovement()->IsWalkable(Hit);
	}
}

void APlayerCharacter::ApplyDefaultSocketOffset()
{
	SpringArmComponent->SocketOffset.Y = DefaultSocketOffsetY;
	NormalSocketOffsetY                 = DefaultSocketOffsetY;
}

void APlayerCharacter::UpdateCoverPeek(float DeltaTime)
{
	float TargetY = NormalSocketOffsetY;
	float TargetZ = NormalSocketOffsetZ;

	if (bIsAiming)
	{
		// 조준 중 카메라 추가 이동 (오른쪽·위) — 아래 엄폐 좌우 이동은 이 위치 기준으로 더해진다
		TargetY += AimSocketOffsetRight;
		TargetZ += AimSocketOffsetUp;

		FVector Origin = GetActorLocation();
		FVector Right  = FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::Y);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		FHitResult LeftHit, RightHit;
		bool bLeftCover  = GetWorld()->LineTraceSingleByChannel(LeftHit,  Origin, Origin - Right * CoverTraceDistance, ECC_WorldStatic, Params);
		bool bRightCover = GetWorld()->LineTraceSingleByChannel(RightHit, Origin, Origin + Right * CoverTraceDistance, ECC_WorldStatic, Params);

		if (bLeftCover && !bRightCover)
			TargetY += CoverPeekOffset;
		else if (bRightCover && !bLeftCover)
			TargetY -= CoverPeekOffset;
	}

	SpringArmComponent->SocketOffset.Y = FMath::FInterpTo(
		SpringArmComponent->SocketOffset.Y, TargetY, DeltaTime, CoverPeekInterpSpeed);
	SpringArmComponent->SocketOffset.Z = FMath::FInterpTo(
		SpringArmComponent->SocketOffset.Z, TargetZ, DeltaTime, CoverPeekInterpSpeed);
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
	MoveInputAxis = Axis;
	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Axis.Y);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Axis.X);
	}
}

void APlayerCharacter::HandleMoveEnd()
{
	MoveInputAxis = FVector2D::ZeroVector;
}

// --- Dodge ---

bool APlayerCharacter::CanStartDodge() const
{
	if (!bCanDodge || bIsDodging) return false;
	if (bIsPreparingThrow || HeldGrenade) return false;   // 수류탄을 손에 든 채로는 회피 없음
	return true;                                          // 공중에서도 가능 (회피 동안은 중력이 멈춘다)
}

float APlayerCharacter::GetDodgeCooldownRemaining() const
{
	if (bCanDodge) return 0.f;
	return FMath::Max(DodgeReadyTime - GetWorld()->GetTimeSeconds(), 0.f);
}

FVector APlayerCharacter::GetDodgeDirection(bool bAimSet, UAnimMontage*& OutMontage) const
{
	// 대각선 입력은 큰 축으로 스냅 — 방향별 몽타주가 4개뿐이라 몸과 애니가 어긋나지 않게
	FVector2D Axis = MoveInputAxis;
	if (Axis.IsNearlyZero())
	{
		Axis = FVector2D(0.f, -1.f);   // 방향 입력 없이 누르면 뒤로
	}
	else if (FMath::Abs(Axis.Y) >= FMath::Abs(Axis.X))
	{
		Axis = FVector2D(0.f, FMath::Sign(Axis.Y));
	}
	else
	{
		Axis = FVector2D(FMath::Sign(Axis.X), 0.f);
	}

	if (bAimSet)
	{
		OutMontage = (Axis.Y > 0.f) ? DodgeAimMontageForward
		           : (Axis.Y < 0.f) ? DodgeAimMontageBackward
		           : (Axis.X > 0.f) ? DodgeAimMontageRight
		                            : DodgeAimMontageLeft;
	}
	if (!OutMontage)   // 조준용이 아직 없으면 일반 세트로
	{
		OutMontage = (Axis.Y > 0.f) ? DodgeMontageForward
		           : (Axis.Y < 0.f) ? DodgeMontageBackward
		           : (Axis.X > 0.f) ? DodgeMontageRight
		                            : DodgeMontageLeft;
	}

	const FRotationMatrix YawMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f));
	return (YawMatrix.GetUnitAxis(EAxis::X) * Axis.Y + YawMatrix.GetUnitAxis(EAxis::Y) * Axis.X).GetSafeNormal();
}

void APlayerCharacter::StartDodge()
{
	if (!CanStartDodge()) return;

	// 조준 중이면 총을 든 채 구르는 세트 — 하체만 몽타주, 상체는 조준 유지
	const bool bAimDodge = bIsAiming;

	UAnimMontage* Montage = nullptr;
	const FVector DodgeDir = GetDodgeDirection(bAimDodge, Montage);

	StopFire();   // 회피 중에는 공격 불가 — 누르고 있던 연사도 끊는다

	bIsDodging       = true;
	bCanDodge        = false;
	bIsAimDodging    = bAimDodge;
	bFullBodyMontage = !bAimDodge;   // 일반 회피만 전신 몽타주 (ABP 전신 분기)

	// 몸은 카메라 방향에 고정 — 옆·뒤로 굴러도 방향별 애니가 제대로 보인다 (EndDodge에서 원복)
	SetActorRotation(FRotator(0.f, GetControlRotation().Yaw, 0.f));
	bUseControllerRotationYaw                         = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// 실제 재생 길이 = 원본 길이 ÷ (입력 배속 × 몽타주 자체 RateScale)
	float MontageLength = 0.f;
	if (Montage)
	{
		const float EffectiveRate = FMath::Max(DodgeMontagePlayRate * Montage->RateScale, KINDA_SMALL_NUMBER);
		MontageLength = PlayAnimMontage(Montage, DodgeMontagePlayRate) / EffectiveRate;
	}
	const float DodgeTime = FMath::Max((bDodgeDurationFromMontage && MontageLength > 0.f) ? MontageLength : DodgeDuration, KINDA_SMALL_NUMBER);

	// 펀치 대시와 같은 방식 — 낙하 모드는 중력·마찰이 섞여 거리가 들쑥날쑥하므로 비행 모드 + 제동 0
	UCharacterMovementComponent* Move = GetCharacterMovement();
	DodgePrevFrictionFactor = Move->BrakingFrictionFactor;
	DodgePrevBrakingFlying  = Move->BrakingDecelerationFlying;
	DodgeVelocity = DodgeDir * (DodgeDistance / DodgeTime);
	Move->BrakingFrictionFactor     = 0.f;
	Move->BrakingDecelerationFlying = 0.f;
	Move->SetMovementMode(MOVE_Flying);
	Move->Velocity = DodgeVelocity;

	GetWorldTimerManager().SetTimer(DodgeEndTimerHandle, this, &APlayerCharacter::EndDodge, DodgeTime, false);

	// HUD는 회피 동작 + 쿨타임을 한 덩어리로 보여준다 (회피 중에 "준비됨"으로 보이지 않게)
	DodgeUnavailableDuration = DodgeTime + DodgeCooldown;
	DodgeReadyTime           = GetWorld()->GetTimeSeconds() + DodgeUnavailableDuration;
}

void APlayerCharacter::EndDodge()
{
	if (!bIsDodging) return;
	bIsDodging       = false;
	bIsAimDodging    = false;
	bFullBodyMontage = false;
	GetWorldTimerManager().ClearTimer(DodgeEndTimerHandle);

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->BrakingFrictionFactor     = DodgePrevFrictionFactor;
	Move->BrakingDecelerationFlying = DodgePrevBrakingFlying;
	Move->Velocity = FVector::ZeroVector;   // 회피는 그 자리에서 멈춘다
	Move->SetMovementMode(MOVE_Falling);

	if (!bIsAiming)
	{
		bUseControllerRotationYaw = false;
		Move->bOrientRotationToMovement = true;
	}

	if (DodgeCooldown > 0.f)
	{
		GetWorldTimerManager().SetTimer(DodgeCooldownTimerHandle, this, &APlayerCharacter::ResetDodgeCooldown, DodgeCooldown, false);
	}
	else
	{
		bCanDodge = true;
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
	if (!SpringArmComponent) return;

	if (bSmoothCrouchCamera)
	{
		const float TargetZ = bIsCrouched ? CrouchCameraZOffset : 0.f;
		CrouchCameraZ = FMath::FInterpTo(CrouchCameraZ, TargetZ, DeltaTime, CrouchCameraInterpSpeed);
	}

	// 공중 포즈에서는 골반이 캡슐보다 크게 올라간다 — 켜져 있으면 그만큼 카메라 피벗도 올린다
	float FollowTarget = 0.f;
	if (bCameraFollowMesh && bIsInAirPose && GetMesh())
	{
		const float Gap = GetMesh()->GetSocketLocation(CameraFollowMeshBone).Z - GetActorLocation().Z;
		FollowTarget = FMath::Clamp(Gap - CameraFollowBaseGap, -CameraFollowMeshMaxOffset, CameraFollowMeshMaxOffset);
	}
	CameraFollowMeshZ = FMath::FInterpTo(CameraFollowMeshZ, FollowTarget, DeltaTime, CameraFollowMeshInterpSpeed);

	SpringArmComponent->TargetOffset.Z = CrouchCameraZ + CameraFollowMeshZ;
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (!bSmoothCrouchCamera)
		CrouchCameraZ = CrouchCameraZOffset;
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (!bSmoothCrouchCamera)
		CrouchCameraZ = 0.f;
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
	LandedTime       = GetWorld()->GetTimeSeconds();

	OnLanding(bHard);

	Super::Landed(Hit);
}

void APlayerCharacter::NotifyHitConfirmed(bool bHeadshot)
{
	OnHitConfirmed.Broadcast(bHeadshot);

	if (HUDData)
	{
		USoundBase* Sound = bHeadshot ? HUDData->HeadshotSound : HUDData->HitSound;
		// 2D로 재생 — 거리와 무관하게 항상 같은 크기로 들린다
		if (Sound) UGameplayStatics::PlaySound2D(this, Sound, HUDData->HitSoundVolume);
	}
}

void APlayerCharacter::NotifyEnemyKilled()
{
	OnEnemyKilled.Broadcast();

	if (HUDData && HUDData->KillSound)
	{
		UGameplayStatics::PlaySound2D(this, HUDData->KillSound, HUDData->HitSoundVolume);
	}
}

void APlayerCharacter::OnLanding_Implementation(bool bHardLanding)
{
}

void APlayerCharacter::ResetHardLanding()
{
	bIsHardLanding = false;
}

void APlayerCharacter::OnDeath_Implementation()
{
	// TODO(체크포인트 재시작 미구현): 사망 연출·재시작 흐름이 생기면 여기서 호출
	StopFire();
	GetCharacterMovement()->StopMovementImmediately();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
	OnDeathEffect();
}
