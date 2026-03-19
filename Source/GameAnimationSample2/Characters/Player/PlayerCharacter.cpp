// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "WeaponBase.h"
#include "GrenadeBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"

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
	WalkSpeed   = 400.0f;
	SprintSpeed = 700.0f;
	NormalFOV   = 90.0f;
	AimFOV      = 60.0f;
	bIsAiming   = false;

	bUseControllerRotationYaw                              = false;
	GetCharacterMovement()->bOrientRotationToMovement      = true;
	GetCharacterMovement()->MaxWalkSpeed                   = WalkSpeed;

	// Trajectory Spline
	TrajectorySpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrajectorySpline"));
	TrajectorySpline->SetupAttachment(RootComponent);

	// Weapon
	CurrentWeapon      = nullptr;
	CurrentWeaponIndex = -1;
	MaxWeaponSlots     = 3;
	WeaponAttachSocket = TEXT("hand_r");
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

	// Spawn default weapon
	if (DefaultWeaponClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		AWeaponBase* Weapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
		PickupWeapon(Weapon);
	}
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

	FVector ThrowStart    = GetMesh()->GetSocketLocation(WeaponAttachSocket);
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

// --- Movement ---

void APlayerCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void APlayerCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// --- Aim ---

void APlayerCharacter::StartAim()
{
	bIsAiming                                         = true;
	CameraComponent->FieldOfView                      = AimFOV;
	bUseControllerRotationYaw                         = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void APlayerCharacter::StopAim()
{
	bIsAiming                                         = false;
	CameraComponent->FieldOfView                      = NormalFOV;
	bUseControllerRotationYaw                         = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

// --- Weapon ---

bool APlayerCharacter::PickupWeapon(AWeaponBase* Weapon)
{
	if (!Weapon) return false;
	if (WeaponInventory.Num() >= MaxWeaponSlots) return false;

	WeaponInventory.Add(Weapon);
	Weapon->SetOwner(this);
	Weapon->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		WeaponAttachSocket);

	if (WeaponInventory.Num() == 1)
	{
		EquipWeapon(0);
	}
	else
	{
		Weapon->SetActorHiddenInGame(true);
	}

	return true;
}

void APlayerCharacter::EquipWeapon(int32 Index)
{
	if (!WeaponInventory.IsValidIndex(Index)) return;
	if (Index == CurrentWeaponIndex) return;

	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
		CurrentWeapon->SetActorHiddenInGame(true);
	}

	CurrentWeaponIndex = Index;
	CurrentWeapon      = WeaponInventory[Index];
	CurrentWeapon->SetActorHiddenInGame(false);
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
