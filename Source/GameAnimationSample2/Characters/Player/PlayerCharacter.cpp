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

	// 기본적으로 모든 행동 허용
	AllowedActions = 0xFF;

	bUseControllerRotationYaw                              = false;
	GetCharacterMovement()->bOrientRotationToMovement      = true;
	GetCharacterMovement()->MaxWalkSpeed                   = WalkSpeed;

	// Traversal
	TraversalComponent    = CreateDefaultSubobject<UTraversalComponent>(TEXT("TraversalComponent"));
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	// AimArrow — BP 뷰포트에서 머즐 소켓 위치에 배치
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
	WeaponAttachSocket = TEXT("WeaponSocket");
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CameraComponent->FieldOfView         = NormalFOV;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

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

	// Spawn default weapons (최대 MaxWeaponSlots개, 첫 번째 자동 장착)
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
	// 트래버설 가능하면 트래버설 시작 (점프 소비)
	if (TraversalComponent && TraversalComponent->TryTraversal()) return;
	Super::Jump();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC) return;

	// IMC 등록
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

	// IMC에서 이름으로 자동 탐색해 바인딩
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
	Bind(TEXT("Crouch"),  ETriggerEvent::Started,   &APlayerCharacter::StartCrouch);
	Bind(TEXT("Crouch"),  ETriggerEvent::Completed, &APlayerCharacter::StopCrouch);
	Bind(TEXT("Reload"),  ETriggerEvent::Started,   &APlayerCharacter::Reload);
	Bind(TEXT("Weapon1"), ETriggerEvent::Started,   &APlayerCharacter::EquipWeaponSlot1);
	Bind(TEXT("Weapon2"), ETriggerEvent::Started,   &APlayerCharacter::EquipWeaponSlot2);
	Bind(TEXT("Weapon3"), ETriggerEvent::Started,   &APlayerCharacter::EquipWeaponSlot3);
	Bind(TEXT("Throw"),   ETriggerEvent::Started,   &APlayerCharacter::StartGrenadeThrow);
	Bind(TEXT("Throw"),   ETriggerEvent::Completed, &APlayerCharacter::ReleaseGrenadeThrow);
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
	UpdateCoverPeek(DeltaTime);

	if (bIsAiming)
	{
		UpdateAimTurn(DeltaTime);
	}

	UpdateAimSpinePitch(DeltaTime);

	// ── 낙하 상태 추적 (AnimBP에서 bIsFalling, CurrentFallSpeed 읽음) ──
	bIsFalling = GetCharacterMovement()->IsFalling();
	if (bIsFalling)
	{
		const float ZVel = GetVelocity().Z;
		// 상승 중(ZVel > 0)이면 FallSpeed는 0, 하강 중이면 양수 값으로 노출
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
			TargetY = NormalSocketOffsetY + CoverPeekOffset;   // 왼쪽 엄폐 → 오른쪽으로
		else if (bRightCover && !bLeftCover)
			TargetY = NormalSocketOffsetY - CoverPeekOffset;   // 오른쪽 엄폐 → 왼쪽으로
	}

	SpringArmComponent->SocketOffset.Y = FMath::FInterpTo(
		SpringArmComponent->SocketOffset.Y, TargetY, DeltaTime, CoverPeekInterpSpeed);
}

// --- Turn In Place ---

void APlayerCharacter::UpdateAimTurn(float DeltaTime)
{
	const float ControlYaw = GetControlRotation().Yaw;
	const float ActorYaw   = GetActorRotation().Yaw;
	AimYaw = FMath::UnwindDegrees(ControlYaw - ActorYaw);

	// 이동 중에는 즉시 카메라 방향으로 스냅 (AimWalk 블렌드스페이스가 처리)
	if (GetVelocity().SizeSquared2D() > 1.f)
	{
		SetActorRotation(FRotator(0.f, ControlYaw, 0.f));
		AimYaw          = 0.f;
		bIsTurningRight = false;
		bIsTurningLeft  = false;
		return;
	}

	// Turn 진행 중: 액터를 카메라 방향으로 회전
	if (bIsTurningRight || bIsTurningLeft)
	{
		const float Delta     = FMath::UnwindDegrees(ControlYaw - GetActorRotation().Yaw);
		const float RotStep   = TurnRotationSpeed * DeltaTime;
		const float RotAmount = FMath::Sign(Delta) * FMath::Min(FMath::Abs(Delta), RotStep);

		FRotator NewRot = GetActorRotation();
		NewRot.Yaw += RotAmount;
		SetActorRotation(NewRot);

		AimYaw = FMath::UnwindDegrees(ControlYaw - NewRot.Yaw);

		// AimYaw가 충분히 줄어들면 Turn 완료
		if (FMath::Abs(AimYaw) < 5.f)
		{
			bIsTurningRight = false;
			bIsTurningLeft  = false;
		}
		return;
	}

	// 임계값 초과 시 Turn 시작
	if (AimYaw >  TurnRightThreshold) { bIsTurningRight = true; return; }
	if (AimYaw < -TurnLeftThreshold)  { bIsTurningLeft  = true; return; }

	// 임계값 미만: SoftTurnSpeed로 천천히 카메라 따라가기
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

// --- Aim Spine Pitch ---

void APlayerCharacter::UpdateAimSpinePitch(float DeltaTime)
{
	float TargetPitch = 0.f;

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

// --- Grenade ---

void APlayerCharacter::StartGrenadeThrow()
{
	if (!GrenadeClass) return;
	bIsPreparingThrow = true;
}

void APlayerCharacter::ReleaseGrenadeThrow()
{
	if (!bIsPreparingThrow) return;
	bIsPreparingThrow = false;
	ClearTrajectory();

	if (!GrenadeClass) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector ThrowStart    = GetMesh()->GetSocketLocation(WeaponAttachSocket);
	FVector ThrowVelocity = ViewRotation.Vector() * GrenadeThrowSpeed;

	FActorSpawnParameters Params;
	Params.Owner     = this;
	Params.Instigator = GetInstigator();

	AGrenadeBase* Grenade = GetWorld()->SpawnActor<AGrenadeBase>(
		GrenadeClass, ThrowStart, ViewRotation, Params);
	if (Grenade)
	{
		Grenade->Launch(ThrowVelocity);
	}
}

void APlayerCharacter::UpdateTrajectory()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector ThrowStart    = ViewLocation + ViewRotation.Vector() * 50.f;
	FVector ThrowVelocity = ViewRotation.Vector() * GrenadeThrowSpeed;

	FPredictProjectilePathParams PredictParams(
		5.f, ThrowStart, ThrowVelocity, 3.f, ECC_WorldDynamic, this);
	PredictParams.bTraceWithCollision = true;
	PredictParams.SimFrequency        = 15.f;

	FPredictProjectilePathResult Result;
	UGameplayStatics::PredictProjectilePath(this, PredictParams, Result);

	// 스플라인 포인트 갱신
	TrajectorySpline->ClearSplinePoints(false);
	for (const FPredictProjectilePathPointData& PointData : Result.PathData)
	{
		TrajectorySpline->AddSplinePoint(PointData.Location, ESplineCoordinateSpace::World, false);
	}
	TrajectorySpline->UpdateSpline();

	// 스플라인 메시 풀 갱신
	int32 NumSegments = FMath::Min(TrajectorySpline->GetNumberOfSplinePoints() - 1, MaxTrajectorySegments);

	for (int32 i = 0; i < SplineMeshPool.Num(); ++i)
	{
		USplineMeshComponent* SMC = SplineMeshPool[i];
		if (i < NumSegments)
		{
			if (TrajectoryMesh && SMC->GetStaticMesh() != TrajectoryMesh)
			{
				SMC->SetStaticMesh(TrajectoryMesh);
				if (TrajectoryMaterial) SMC->SetMaterial(0, TrajectoryMaterial);
			}

			FVector StartPos, StartTangent, EndPos, EndTangent;
			TrajectorySpline->GetLocationAndTangentAtSplinePoint(i,     StartPos, StartTangent, ESplineCoordinateSpace::World);
			TrajectorySpline->GetLocationAndTangentAtSplinePoint(i + 1, EndPos,   EndTangent,   ESplineCoordinateSpace::World);

			float SegmentLength = FVector::Dist(StartPos, EndPos);
			StartTangent = StartTangent.GetSafeNormal() * SegmentLength;
			EndTangent   = EndTangent.GetSafeNormal()   * SegmentLength;

			// 컴포넌트를 시작점으로 이동 후 로컬 상대좌표로 넘김
			SMC->SetWorldLocation(StartPos);
			SMC->SetWorldRotation(FRotator::ZeroRotator);
			SMC->SetStartAndEnd(FVector::ZeroVector, StartTangent, EndPos - StartPos, EndTangent);
			SMC->SetStartScale(FVector2D(TrajectoryMeshScale, TrajectoryMeshScale));
			SMC->SetEndScale(FVector2D(TrajectoryMeshScale, TrajectoryMeshScale));
			SMC->SetVisibility(true);
		}
		else
		{
			SMC->SetVisibility(false);
		}
	}
}

void APlayerCharacter::ClearTrajectory()
{
	for (USplineMeshComponent* SMC : SplineMeshPool)
	{
		SMC->SetVisibility(false);
	}
	TrajectorySpline->ClearSplinePoints();
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

// --- Action Permission ---

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

// --- Movement ---

void APlayerCharacter::StartSprint()
{
	if (!CanDo(ECharacterAction::Sprint)) return;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void APlayerCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : WalkSpeed;
}

// --- Aim ---

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

// --- Weapon ---

bool APlayerCharacter::PickupWeapon(AWeaponBase* Weapon)
{
	if (!Weapon) return false;
	if (WeaponInventory.Num() >= MaxWeaponSlots) return false;

	WeaponInventory.Add(Weapon);
	Weapon->SetOwner(this);

	// 첫 무기면 바로 장착, 아니면 홀스터 소켓에 수납
	if (WeaponInventory.Num() == 1)
	{
		Weapon->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			WeaponAttachSocket);
		CurrentWeaponIndex = 0;
		CurrentWeapon      = Weapon;
	}
	else
	{
		const FName HolsterSocket = WeaponHolsterSocket.IsNone() ? WeaponAttachSocket : WeaponHolsterSocket;
		Weapon->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			HolsterSocket);
		Weapon->SetActorHiddenInGame(WeaponHolsterSocket.IsNone());
	}

	return true;
}

void APlayerCharacter::EquipWeapon(int32 Index)
{
	if (!WeaponInventory.IsValidIndex(Index)) return;
	if (Index == CurrentWeaponIndex) return;
	if (bIsSwapping) return;

	bIsSwapping        = true;
	PendingWeaponIndex = Index;

	// 현재 무기 홀스터로 이동
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
		const FName HolsterSocket = WeaponHolsterSocket.IsNone() ? WeaponAttachSocket : WeaponHolsterSocket;
		CurrentWeapon->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			HolsterSocket);
		if (WeaponHolsterSocket.IsNone())
			CurrentWeapon->SetActorHiddenInGame(true);
	}

	GetWorldTimerManager().SetTimer(SwapTimerHandle, this, &APlayerCharacter::FinishEquipWeapon, WeaponSwapDelay, false);
}

void APlayerCharacter::FinishEquipWeapon()
{
	bIsSwapping = false;

	if (!WeaponInventory.IsValidIndex(PendingWeaponIndex)) return;

	CurrentWeaponIndex = PendingWeaponIndex;
	CurrentWeapon      = WeaponInventory[CurrentWeaponIndex];

	CurrentWeapon->SetActorHiddenInGame(false);
	CurrentWeapon->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		WeaponAttachSocket);
}

void APlayerCharacter::StartCrouch() { Crouch(); }
void APlayerCharacter::StopCrouch()  { UnCrouch(); }

void APlayerCharacter::EquipWeaponSlot1() { EquipWeapon(0); }
void APlayerCharacter::EquipWeaponSlot2() { EquipWeapon(1); }
void APlayerCharacter::EquipWeaponSlot3() { EquipWeapon(2); }

void APlayerCharacter::StartFire()
{
	if (CurrentWeapon) CurrentWeapon->StartFire();
}

void APlayerCharacter::StopFire()
{
	if (CurrentWeapon) CurrentWeapon->StopFire();
}

void APlayerCharacter::Reload()
{
	if (CurrentWeapon) CurrentWeapon->Reload();
}

// --- Gravity ---

void APlayerCharacter::SetGravityDirection(FVector NewDirection)
{
	if (NewDirection.IsNearlyZero()) return;
	GetCharacterMovement()->SetGravityDirection(NewDirection.GetSafeNormal());
}

void APlayerCharacter::ResetGravity()
{
	GetCharacterMovement()->SetGravityDirection(FVector(0.f, 0.f, -1.f));
}

// --- Fall / Landing ---

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	// Landed()는 실제 착지 직전 프레임의 속도를 가지고 있지 않으므로
	// Tick에서 추적한 CurrentFallSpeed를 착지 판정에 사용한다.
	const float LandingSpeed = CurrentFallSpeed;
	const bool  bHard        = LandingSpeed >= HardLandingSpeedThreshold;

	CurrentFallSpeed = 0.f;
	bIsFalling       = false;
	bIsHardLanding   = bHard;

	// AnimBP 이벤트 발동
	OnLanding(bHard);

	Super::Landed(Hit);
}

void APlayerCharacter::OnLanding_Implementation(bool bHardLanding)
{
	// BP에서 오버라이드:
	//   if bHardLanding → PlayAnimMontage(HardLandMontage)
	//   else            → PlayAnimMontage(SoftLandMontage)
	// 몽타주 끝 Anim Notify에서 ResetHardLanding() 호출
}

void APlayerCharacter::ResetHardLanding()
{
	bIsHardLanding = false;
}
