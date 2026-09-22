// Copyright Epic Games, Inc. All Rights Reserved.

#include "MikaCharacter.h"
#include "IDestructible.h"
#include "HealthRegenComponent.h"
#include "WeaponBase.h"
#include "GrenadeBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"

// Vefects 팩은 월드 공간 이미터가 많아 컴포넌트 스케일이 전부 먹지 않는다 — 노출된 User.Scale Overall이 있으면 그걸 쓴다
static const FName HandVFXScaleParam(TEXT("User.Scale Overall"));

static bool HasScaleOverallParam(const UNiagaraSystem* System)
{
	return System && System->GetExposedParameters().FindParameterOffset(
		FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), HandVFXScaleParam)) != nullptr;
}

AMikaCharacter::AMikaCharacter()
{
	HealthRegen = CreateDefaultSubobject<UHealthRegenComponent>(TEXT("HealthRegen"));

	// 몸 앞에 내민 캡슐 판정. 크기·오프셋은 BeginPlay에서 MikaData 값으로 덮어쓴다
	PunchHitCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("PunchHitCapsule"));
	PunchHitCapsule->SetupAttachment(GetCapsuleComponent());
	PunchHitCapsule->InitCapsuleSize(PunchHitRadius, PunchHitHalfHeight);
	PunchHitCapsule->SetRelativeLocation(FVector(PunchHitForwardOffset, 0.f, 0.f));
	PunchHitCapsule->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	PunchHitCapsule->SetGenerateOverlapEvents(false);
	PunchHitCapsule->SetCanEverAffectNavigation(false);

	// 범위 데칼은 캐릭터 회전과 무관하게 월드 기준으로 매 프레임 배치
	PunchRangeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("PunchRangeDecal"));
	PunchRangeDecal->SetupAttachment(RootComponent);
	PunchRangeDecal->SetUsingAbsoluteLocation(true);
	PunchRangeDecal->SetUsingAbsoluteRotation(true);
	PunchRangeDecal->SetUsingAbsoluteScale(true);
	PunchRangeDecal->SetHiddenInGame(true);

	PunchExplosionDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("PunchExplosionDecal"));
	PunchExplosionDecal->SetupAttachment(RootComponent);
	PunchExplosionDecal->SetUsingAbsoluteLocation(true);
	PunchExplosionDecal->SetUsingAbsoluteRotation(true);
	PunchExplosionDecal->SetUsingAbsoluteScale(true);
	PunchExplosionDecal->SetHiddenInGame(true);
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
		CrouchWalkSpeed            = MikaData->CrouchWalkSpeed;
		GetCharacterMovement()->MaxWalkSpeed         = WalkSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchWalkSpeed;
		// Camera
		NormalFOV                  = MikaData->NormalFOV;
		CrouchCameraZOffset        = MikaData->CrouchCameraZOffset;
		bSmoothCrouchCamera        = MikaData->bSmoothCrouchCamera;
		CrouchCameraInterpSpeed    = MikaData->CrouchCameraInterpSpeed;
		AimFOV                     = MikaData->AimFOV;
		CoverTraceDistance         = MikaData->CoverTraceDistance;
		CoverPeekOffset            = MikaData->CoverPeekOffset;
		CoverPeekInterpSpeed       = MikaData->CoverPeekInterpSpeed;
		DefaultSocketOffsetY       = MikaData->DefaultSocketOffsetY;
			AimSpinePitchClamp         = MikaData->AimSpinePitchClamp;
		CameraPitchMin             = MikaData->CameraPitchMin;
		CameraPitchMax             = MikaData->CameraPitchMax;
		ChargeCameraPitchMin       = MikaData->ChargeCameraPitchMin;
		AimSocketOffsetRight       = MikaData->AimSocketOffsetRight;
		AimSocketOffsetUp          = MikaData->AimSocketOffsetUp;
		AimWaistYawOffset          = MikaData->AimWaistYawOffset;
		AimWaistBlendSpeed         = MikaData->AimWaistBlendSpeed;
			AimSpineInterpSpeed        = MikaData->AimSpineInterpSpeed;
			// TurnInPlace
		TurnRightThreshold         = MikaData->TurnRightThreshold;
		TurnLeftThreshold          = MikaData->TurnLeftThreshold;
		TurnRotationSpeed          = MikaData->TurnRotationSpeed;
		SoftTurnSpeed              = MikaData->SoftTurnSpeed;
		// Fall
		HardLandingSpeedThreshold  = MikaData->HardLandingSpeedThreshold;
		LandPoseHoldTime           = MikaData->LandPoseHoldTime;
		LandAnticipationTime       = MikaData->LandAnticipationTime;
		AirPoseBlendSpeed          = MikaData->AirPoseBlendSpeed;
		JumpAnimPlayRate           = MikaData->JumpAnimPlayRate;
		// Grenade / Weapon
		// 비어 있으면 BP_Mika에 직접 넣어둔 값을 유지 (DA 이전 전 기존 설정 보호)
		if (MikaData->GrenadeClass) GrenadeClass = MikaData->GrenadeClass;
		GrenadeThrowSpeed          = MikaData->GrenadeThrowSpeed;
		GrenadeLaunchOffset        = MikaData->GrenadeLaunchOffset;
			GrenadeCount               = MikaData->MaxGrenadeCount;
		WeaponSwapDelay            = MikaData->WeaponSwapDelay;
		CurrentHealth              = MaxHealth;
		HealthRegen->Configure(MikaData->RegenDelay, MikaData->RegenInterval, MikaData->RegenAmount, MikaData->RegenCapRatio);
		// Punch
		PunchCooldown              = MikaData->PunchCooldown;
		ChargeGravityScale         = MikaData->ChargeGravityScale;
		ChargeMovementScale        = MikaData->ChargeMovementScale;
		ChargeBrakingDeceleration  = MikaData->ChargeBrakingDeceleration;
		ChargeAirBrakingDeceleration = MikaData->ChargeAirBrakingDeceleration;
		MinChargeTime              = MikaData->MinChargeTime;
		MaxChargeTime              = MikaData->MaxChargeTime;
		ForcedMaxChargeTime        = MikaData->ForcedMaxChargeTime;
		DashDuration               = MikaData->DashDuration;
		bDashDurationFromMontage   = MikaData->bDashDurationFromMontage;
		DashMaxVisualPitch         = MikaData->DashMaxVisualPitch;
		// Punch Range / Hit
		PunchMinDistance           = MikaData->PunchMinDistance;
		PunchMaxDistance           = MikaData->PunchMaxDistance;
		PunchHitRadius             = MikaData->PunchHitRadius;
		PunchHitHalfHeight         = MikaData->PunchHitHalfHeight;
		PunchHitForwardOffset      = MikaData->PunchHitForwardOffset;
		PunchRangeDecalMaterial    = MikaData->PunchRangeDecalMaterial;
		bDebugDrawPunchHit         = MikaData->bDebugDrawPunchHit;
		PunchMinDamage             = MikaData->PunchMinDamage;
		PunchMaxDamage             = MikaData->PunchMaxDamage;
		PunchFrontHalfWidth        = MikaData->PunchFrontHalfWidth;
		PunchKnockbackPerDistance  = MikaData->PunchKnockbackPerDistance;
		PunchKnockbackUpRatio      = MikaData->PunchKnockbackUpRatio;
		PunchSideDamageRatio       = MikaData->PunchSideDamageRatio;
		PunchSideKnockbackRatio    = MikaData->PunchSideKnockbackRatio;
		PunchReboundDistanceRatio  = MikaData->PunchReboundDistanceRatio;
		PunchReboundTime           = MikaData->PunchReboundTime;
		PunchEndMomentumRatio      = MikaData->PunchEndMomentumRatio;
		PunchHitSlowTimeScale      = MikaData->PunchHitSlowTimeScale;
		PunchHitSlowDuration       = MikaData->PunchHitSlowDuration;
		PunchExplosionMinRadius    = MikaData->PunchExplosionMinRadius;
		PunchExplosionMaxRadius    = MikaData->PunchExplosionMaxRadius;
		PunchExplosionMinDamage    = MikaData->PunchExplosionMinDamage;
		PunchExplosionMaxDamage    = MikaData->PunchExplosionMaxDamage;
		PunchExplosionKnockbackRatio = MikaData->PunchExplosionKnockbackRatio;
		PunchExplosionVFX          = MikaData->PunchExplosionVFX;
		PunchExplosionVFXReferenceRadius = MikaData->PunchExplosionVFXReferenceRadius;
		PunchFullChargeSFX         = MikaData->PunchFullChargeSFX;
		PunchFullChargeHandVFX     = MikaData->PunchFullChargeHandVFX;
		PunchFullChargeHandVFXScale = MikaData->PunchFullChargeHandVFXScale;
		PunchExplosionDecalMaterial = MikaData->PunchExplosionDecalMaterial;
		PunchReboundDelayFull      = MikaData->PunchReboundDelayFull;
		PunchReboundDelayNormal    = MikaData->PunchReboundDelayNormal;
		PunchSlamRadius            = MikaData->PunchSlamRadius;
		PunchSlamMinDownPitch      = MikaData->PunchSlamMinDownPitch;
		// Punch FX
		ChargeHandVFX              = MikaData->ChargeHandVFX;
		ChargeHandVFXMinScale      = MikaData->ChargeHandVFXMinScale;
		ChargeHandVFXMaxScale      = MikaData->ChargeHandVFXMaxScale;
		ChargeHandVFXSourceLength  = MikaData->ChargeHandVFXSourceLength;
		ChargeHandVFXCameraOffset  = MikaData->ChargeHandVFXCameraOffset;
		DashHandVFX                = MikaData->DashHandVFX;
		ChargeHandSFX              = MikaData->ChargeHandSFX;
		DashHandSFX                = MikaData->DashHandSFX;
		PunchHitVFX                = MikaData->PunchHitVFX;
		PunchHitVFXMinScale        = MikaData->PunchHitVFXMinScale;
		PunchHitVFXMaxScale        = MikaData->PunchHitVFXMaxScale;
		PunchHitSFXMinVolume       = MikaData->PunchHitSFXMinVolume;
		PunchHitSFXMaxVolume       = MikaData->PunchHitSFXMaxVolume;
		PunchHitSFX                = MikaData->PunchHitSFX;
		PunchSideHitVFX            = MikaData->PunchSideHitVFX;
		PunchSideHitVFXScale       = MikaData->PunchSideHitVFXScale;
		PunchSideHitSFX            = MikaData->PunchSideHitSFX;
		// Punch Camera
		ChargeSpringArmLength      = MikaData->ChargeSpringArmLength;
		ChargeFOV                  = MikaData->ChargeFOV;
		DashSpringArmLength        = MikaData->DashSpringArmLength;
		DashFOV                    = MikaData->DashFOV;
		CameraInterpSpeed          = MikaData->CameraInterpSpeed;
		ChargeShakeMaxLocation     = MikaData->ChargeShakeMaxLocation;
		ChargeShakeMaxRotation     = MikaData->ChargeShakeMaxRotation;
		ChargeShakeFrequency       = MikaData->ChargeShakeFrequency;
		FireMontage                = MikaData->FireMontage;
		FireMontageCrouch          = MikaData->FireMontageCrouch;
		GrenadePrepareMontage      = MikaData->GrenadePrepareMontage;
		GrenadeThrowMontage        = MikaData->GrenadeThrowMontage;
		ReloadMontage              = MikaData->ReloadMontage;
		PunchChargeMontage         = MikaData->PunchChargeMontage;
		PunchDashMontage           = MikaData->PunchDashMontage;
	}

	PunchHitCapsule->SetCapsuleSize(PunchHitRadius, PunchHitHalfHeight);
	PunchHitCapsule->SetRelativeLocation(FVector(PunchHitForwardOffset, 0.f, 0.f));
	if (PunchRangeDecalMaterial)
	{
		PunchRangeDecal->SetDecalMaterial(PunchRangeDecalMaterial);
		PunchRangeDecalMID = PunchRangeDecal->CreateDynamicMaterialInstance();
		if (PunchRangeDecalMID)
		{
			PunchRangeDecalFillOpacity = PunchRangeDecalMID->K2_GetScalarParameterValue(TEXT("FillOpacity"));
		}
	}
	if (PunchExplosionDecalMaterial)
	{
		PunchExplosionDecal->SetDecalMaterial(PunchExplosionDecalMaterial);
		PunchExplosionDecalMID = PunchExplosionDecal->CreateDynamicMaterialInstance();
		if (PunchExplosionDecalMID)
		{
			PunchExplosionDecalFillOpacity = PunchExplosionDecalMID->K2_GetScalarParameterValue(TEXT("FillOpacity"));
		}
	}

	ApplyCameraPitchLimits();
	ApplyDefaultSocketOffset();   // 부모 BeginPlay는 DA 복사 전이라 BP 기본값이 들어가 있었음 — DA 값으로 다시 적용
	PunchHitCapsule->OnComponentBeginOverlap.AddDynamic(this, &AMikaCharacter::OnPunchHitboxOverlap);
	NormalSpringArmLength        = SpringArmComponent->TargetArmLength;
	CameraBaseRelLocation        = CameraComponent->GetRelativeLocation();
	CameraBaseRelRotation        = CameraComponent->GetRelativeRotation();
	DefaultBrakingDeceleration     = GetCharacterMovement()->BrakingDecelerationWalking;
	DefaultBrakingDecelerationFly  = GetCharacterMovement()->BrakingDecelerationFlying;
	DefaultBrakingDecelerationFall = GetCharacterMovement()->BrakingDecelerationFalling;
	DefaultBrakingFrictionFactor   = GetCharacterMovement()->BrakingFrictionFactor;
}

void AMikaCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ApplyCameraPitchLimits();
}

void AMikaCharacter::ApplyCameraPitchLimits()
{
	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->PlayerCameraManager)
	{
		// 충전 중엔 바닥을 내려찍을 수 있게 더 아래까지 허용
		PC->PlayerCameraManager->ViewPitchMin = bIsChargingPunch ? ChargeCameraPitchMin : CameraPitchMin;
		PC->PlayerCameraManager->ViewPitchMax = CameraPitchMax;
	}
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

	// 조준 중 허리를 더 틀기 (ABP가 AimWaistYaw를 Spine1에 더함)
	AimWaistYaw = FMath::FInterpTo(AimWaistYaw, bIsAiming ? AimWaistYawOffset : 0.f, DeltaTime, AimWaistBlendSpeed);

	// 충전 중 캐릭터가 카메라 yaw 방향을 바라봄
	if (bIsChargingPunch)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FRotator CtrlRot = PC->GetControlRotation();
			SetActorRotation(FRotator(0.f, CtrlRot.Yaw, 0.f));
		}
		const float ChargeRatio = GetChargeRatio();
		if (!bFullChargeNotified && ChargeRatio >= 1.f)
		{
			// 풀 충전 도달 — UI 알림 이벤트 + 사운드, 범위 데칼 외곽선 → 채움
			bFullChargeNotified = true;
			if (PunchFullChargeSFX) UGameplayStatics::PlaySound2D(this, PunchFullChargeSFX);
			if (PunchFullChargeHandVFX)
			{
				StopFullChargeVFX();
				ActiveFullChargeVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
					PunchFullChargeHandVFX, GetMesh(), TEXT("WeaponSocketRight"),
					FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
				if (ActiveFullChargeVFX && HasScaleOverallParam(PunchFullChargeHandVFX))
				{
					ActiveFullChargeVFX->SetVariableFloat(HandVFXScaleParam, PunchFullChargeHandVFXScale);
				}
				else if (ActiveFullChargeVFX)
				{
					ActiveFullChargeVFX->SetWorldScale3D(FVector(PunchFullChargeHandVFXScale));
				}
			}
			SetPunchRangeDecalFilled(true);
			OnPunchFullCharge.Broadcast();
		}
		SetChargeVFXScale(ChargeRatio);
		UpdateChargeVFXCameraOffset();
		UpdatePunchRangeDecal(ChargeRatio);
		UpdateChargeShake(ChargeRatio);
	}

	// 대시 중엔 매 프레임 같은 속도를 유지 — 벽에 막혀 미끄러져도 다시 원래 방향으로
	if (bIsDashing && bPendingSlam)
	{
		bPendingSlam = false;
		PunchSlam(PendingSlamPoint);
	}
	else if (bIsDashing && bPendingRebound)
	{
		bPendingRebound = false;
		ReboundFromHit();
	}
	else if (bIsDashing)
	{
		// 바닥 목표 대시: 목표까지 이동했으면 그 지점에서 착지 공격 (지상 출발은 수평 거리로 판정)
		FVector Moved = GetActorLocation() - DashStartLocation;
		if (bDashFloorFlat) Moved.Z = 0.f;
		if (bDashFloorTarget && Moved.Size() >= DashFloorTravel)
		{
			PunchSlam(DashFloorTarget);
		}
		else
		{
			GetCharacterMovement()->Velocity = DashVelocity;
		}
	}
	else if (bIsRebounding)
	{
		GetCharacterMovement()->Velocity = FVector::ZeroVector;   // 반동 딜레이 동안 제자리
	}

	if (bDebugDrawPunchHit && (bIsChargingPunch || bIsDashing))
	{
		DrawDebugCapsule(GetWorld(), PunchHitCapsule->GetComponentLocation(),
			PunchHitCapsule->GetScaledCapsuleHalfHeight(), PunchHitCapsule->GetScaledCapsuleRadius(),
			PunchHitCapsule->GetComponentQuat(), bIsDashing ? FColor::Red : FColor::Yellow);
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

	// 조준 중 발사하다 조준을 풀고 펀치로 넘어오면 무기 타이머가 계속 돌 수 있다 — 충전 진입 시 확실히 멈춘다
	Super::StopFire();

	bIsChargingPunch = true;
	ChargeStartTime  = GetWorld()->GetTimeSeconds();
	// 충전 중엔 몸이 카메라를 보게 고정 (Tick에서 yaw 적용). 이동 방향으로 돌면 옆으로 걸어도 앞으로 걷는 애니가 나온다
	GetCharacterMovement()->bOrientRotationToMovement = false;
	ApplyCameraPitchLimits();   // 충전용 아래 각도로 확장
	GetCharacterMovement()->GravityScale                 = ChargeGravityScale;
	GetCharacterMovement()->MaxWalkSpeed                 = WalkSpeed * ChargeMovementScale;
	GetCharacterMovement()->BrakingDecelerationWalking   = ChargeBrakingDeceleration;
	GetCharacterMovement()->BrakingDecelerationFalling   = ChargeAirBrakingDeceleration;
	GetWorldTimerManager().SetTimer(AutoReleaseTimerHandle, this, &AMikaCharacter::StopFire, ForcedMaxChargeTime, false);

	GetWorldTimerManager().ClearTimer(WeaponRestoreTimerHandle);
	SetCurrentWeaponHidden(true);
	if (PunchChargeMontage) PlayAnimMontage(PunchChargeMontage);
	StartHandVFX(ChargeHandVFX);
	SetChargeVFXScale(0.f);
	UpdateChargeVFXCameraOffset();   // 첫 프레임부터 앞에 보이게
	// 짧은 캐스트 이펙트를 강제 발동 시간까지 늘려 1회 재생 (0.5초 원본, 2초 충전 → 0.25배속)
	if (ActiveHandVFX && ChargeHandVFXSourceLength > 0.f && ForcedMaxChargeTime > 0.f)
	{
		ActiveHandVFX->SetCustomTimeDilation(ChargeHandVFXSourceLength / ForcedMaxChargeTime);
	}
	PlayHandSFX(ChargeHandSFX);
	bFullChargeNotified = false;
	StopFullChargeVFX();
	SetPunchRangeDecalFilled(false);
	UpdatePunchRangeDecal(0.f);   // 데칼 표시 여부도 여기서 결정
}

void AMikaCharacter::StopFire()
{
	if (!bIsChargingPunch)
	{
		// 충전 중이 아니면 사격 릴리즈. 조준을 푼 채 버튼을 떼도 총구가 멈춰야 한다
		Super::StopFire();
		return;
	}

	bIsChargingPunch = false;
	ApplyCameraPitchLimits();   // 평소 아래 각도로 복귀
	GetWorldTimerManager().ClearTimer(AutoReleaseTimerHandle);
	GetCharacterMovement()->GravityScale                 = 1.f;
	GetCharacterMovement()->MaxWalkSpeed                 = WalkSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking   = DefaultBrakingDeceleration;
	GetCharacterMovement()->BrakingDecelerationFalling   = DefaultBrakingDecelerationFall;

	if (PunchChargeMontage) StopAnimMontage(PunchChargeMontage);
	StopHandVFX();   // 충전 FX 정지 — 대시로 이어지면 StartDash에서 대시 FX 재생
	StopHandSFX();
	PunchRangeDecal->SetHiddenInGame(true);
	PunchExplosionDecal->SetHiddenInGame(true);
	StopChargeShake();

	// 비율은 충전 시간 기준이라 bIsChargingPunch와 무관하게 계산된다
	const float ChargeRatio = GetChargeRatio();
	float HeldTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	if (HeldTime < MinChargeTime)
	{
		SetCurrentWeaponHidden(false);   // 미발동 — 총 복구
		// 대시로 이어지지 않으므로 이동 방향 회전을 되돌린다 (대시는 EndDash에서 복구)
		if (!bIsAiming) GetCharacterMovement()->bOrientRotationToMovement = true;
		return;
	}

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

// --- 대시 ---

FVector AMikaCharacter::GetPunchAimDirection() const
{
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FVector Loc; FRotator Rot;
		PC->GetPlayerViewPoint(Loc, Rot);
		return Rot.Vector();
	}
	return GetActorForwardVector();
}

void AMikaCharacter::StartDash(float ChargeRatio)
{
	HitActorsDuringDash.Empty();
	bDashHitFXPlayed = false;
	bPendingRebound  = false;
	bPendingSlam     = false;
	bIsDashing       = true;
	bCanPunch        = false;
	DashChargeRatio  = ChargeRatio;
	bDashFullCharge  = ChargeRatio >= 1.f;   // MaxChargeTime 이상 눌렀을 때만 메인 히트 연출

	const FVector DashDir = GetPunchAimDirection();

	// 몸은 펀치를 시작한 방향에 고정 — 대시 중 카메라를 돌려도 따라 돌지 않는다 (EndDash에서 원복)
	SetActorRotation(FRotator(0.f, DashDir.Rotation().Yaw, 0.f));
	bUseControllerRotationYaw                         = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	DashPitch = FMath::Clamp(DashDir.Rotation().Pitch, -DashMaxVisualPitch, DashMaxVisualPitch);

	StartHandVFX(DashHandVFX);
	PlayHandSFX(DashHandSFX);
	const float DashMontageLength = PlayMontageForDuration(PunchDashMontage);
	PunchMontageEndTime = GetWorld()->GetTimeSeconds() + DashMontageLength;

	// 반동으로 대시가 일찍 끝나도 몽타주 끝까지 하반신을 펀치 모션으로 (몽타주가 없으면 대시 끝에서 해제)
	bIsPunchFullBody = true;
	GetWorldTimerManager().ClearTimer(PunchFullBodyTimerHandle);

	// 거리 = 충전 비율로 Min~Max, 시간 = 몽타주 길이(또는 DashDuration) → 속도 = 거리 ÷ 시간.
	// 낙하 모드는 중력·공중 감속이 섞여 거리가 들쑥날쑥했으므로 비행 모드 + 제동 0으로 일정 속도 직선 이동
	const float DashTotalTime = FMath::Max((bDashDurationFromMontage && DashMontageLength > 0.f) ? DashMontageLength : DashDuration, KINDA_SMALL_NUMBER);
	DashDistance = FMath::Lerp(PunchMinDistance, PunchMaxDistance, ChargeRatio);

	UCharacterMovementComponent* Move = GetCharacterMovement();

	// 범위 원과 같은 트레이스로 목표를 정한다 — 경로가 바닥에 닿으면(원이 바닥에 뜨면) 그 지점까지 가서 착지 공격.
	// 지상 출발이면 바로 바닥에 막혀 미끄러지지 않도록 바닥을 따라 수평으로 이동
	FVector MoveDir;
	FVector PathEnd, PathNormal;
	DashStartLocation = GetActorLocation();
	bDashFloorTarget  = TracePunchPath(ChargeRatio, PathEnd, MoveDir, PathNormal) == EPunchPathEnd::Floor;
	bDashFloorFlat    = false;
	if (bDashFloorTarget)
	{
		DashFloorTarget = PathEnd;
		FVector ToTarget = PathEnd - DashStartLocation;
		if (Move->IsMovingOnGround())
		{
			bDashFloorFlat = true;
			ToTarget.Z = 0.f;
			MoveDir = ToTarget.GetSafeNormal();   // 바로 아래를 보면 0 → 제자리 착지 공격
		}
		DashFloorTravel = ToTarget.Size();
	}
	DashVelocity = MoveDir * (DashDistance / DashTotalTime);

	Move->BrakingFrictionFactor     = 0.f;
	Move->BrakingDecelerationFlying = 0.f;
	Move->SetMovementMode(MOVE_Flying);
	Move->Velocity = DashVelocity;

	GetWorldTimerManager().SetTimer(DashEndTimerHandle, this, &AMikaCharacter::EndDashKeepMomentum, DashTotalTime, false);
	GetWorldTimerManager().SetTimer(PunchFullBodyTimerHandle, this, &AMikaCharacter::EndPunchFullBody,
		FMath::Max(DashMontageLength, DashTotalTime), false);

	// 판정은 마지막에 켠다 — 이미 겹쳐 있는 적이 즉시 맞아 EndDash가 불려도 위 설정이 모두 끝난 뒤여야 한다.
	// 켜는 것만으로는 이미 겹친 대상의 시작 이벤트가 안 나오므로 즉시 갱신
	PunchHitCapsule->SetGenerateOverlapEvents(true);
	PunchHitCapsule->UpdateOverlaps();
}

void AMikaCharacter::EndPunchFullBody()
{
	bIsPunchFullBody = false;
	DashPitch        = 0.f;
}

void AMikaCharacter::EndDash()
{
	FinishDash(false);
}

void AMikaCharacter::EndDashKeepMomentum()
{
	FinishDash(true);
}

void AMikaCharacter::FinishDash(bool bKeepMomentum)
{
	if (!bIsDashing) return;   // 적중 반동·착지 공격으로 이미 끝난 대시의 타이머
	bIsDashing = false;
	// DashPitch(몸 기울기)는 여기서 풀지 않는다 — 바닥을 보고 치면 시작하자마자 착지 공격으로 대시가 끝나
	// 기울기가 한 프레임 만에 0이 되어 수평 펀치(카메라 기준 위를 보는 펀치)로 보였다. 몽타주 끝(EndPunchFullBody)에서 해제
	GetWorldTimerManager().ClearTimer(DashEndTimerHandle);
	PunchHitCapsule->SetGenerateOverlapEvents(false);
	StopHandVFX();
	StopHandSFX();
	StopFullChargeVFX();
	HitActorsDuringDash.Empty();

	// 낙하 모드로 복귀. 끝까지 간 대시는 속도를 이어가고(지상은 걷기 제동으로 미끄러지며 감속, 공중은 착지까지 유지),
	// 적중·벽·착지 공격으로 끝나면 그 자리 정지 (이후 반동이 이어질 수 있음)
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->BrakingFrictionFactor     = DefaultBrakingFrictionFactor;
	Move->BrakingDecelerationFlying = DefaultBrakingDecelerationFly;
	Move->Velocity = bKeepMomentum ? DashVelocity * PunchEndMomentumRatio : FVector::ZeroVector;
	Move->SetMovementMode(MOVE_Falling);

	if (!bIsAiming)
	{
		bUseControllerRotationYaw                         = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	RestoreWeaponAfter(PunchMontageEndTime - GetWorld()->GetTimeSeconds());
	StartPunchCooldown();
}

// --- 적중 ---

void AMikaCharacter::ApplyPunchHit(AActor* Target, UPrimitiveComponent* TargetComp, const FVector& KnockbackDir, float DamageScale, float KnockbackScale)
{
	if (!Target) return;

	// IDestructibleObject이면 즉각 파괴
	if (IDestructibleObject* Destructible = Cast<IDestructibleObject>(Target))
	{
		Destructible->Execute_DestroyByPunch(Target, this);
		return;
	}

	const float Damage = FMath::Lerp(PunchMinDamage, PunchMaxDamage, DashChargeRatio) * DamageScale;
	UGameplayStatics::ApplyDamage(Target, Damage, GetController(), this, UDamageType::StaticClass());

	// 넉백 세기는 이번 대시 거리에 비례
	const float Speed = DashDistance * PunchKnockbackPerDistance * KnockbackScale;
	const FVector Knockback = KnockbackDir * Speed + FVector::UpVector * Speed * PunchKnockbackUpRatio;

	if (ACharacter* HitCharacter = Cast<ACharacter>(Target))
	{
		HitCharacter->LaunchCharacter(Knockback, true, true);
	}
	else if (TargetComp && TargetComp->IsSimulatingPhysics())
	{
		TargetComp->AddImpulse(Knockback, NAME_None, true);   // 질량 무관 속도 변화
	}
}

// --- 충전 진동 ---

void AMikaCharacter::UpdateChargeShake(float ChargeRatio)
{
	// 축마다 다른 오프셋의 펄린 노이즈(-1~1) × 충전 비율 × 최대 폭. 매 프레임 새로 계산해 감쇠·지연 없이 흔든다
	const float T = (GetWorld()->GetTimeSeconds() - ChargeStartTime) * ChargeShakeFrequency;
	auto Noise = [T](float Seed) { return FMath::PerlinNoise1D(T + Seed); };

	const float Loc = ChargeShakeMaxLocation * ChargeRatio;
	const float Rot = ChargeShakeMaxRotation * ChargeRatio;
	CameraComponent->SetRelativeLocationAndRotation(
		CameraBaseRelLocation + FVector(0.f, Noise(11.3f), Noise(37.7f)) * Loc,
		CameraBaseRelRotation + FRotator(Noise(53.1f) * Rot, Noise(71.9f) * Rot, Noise(97.3f) * Rot));
}

void AMikaCharacter::StopChargeShake()
{
	CameraComponent->SetRelativeLocationAndRotation(CameraBaseRelLocation, CameraBaseRelRotation);
}

bool AMikaCharacter::IsFrontHit(const AActor* Target) const
{
	// 대시 직선(미카 위치, 대시 방향)에서 대상 중심까지의 수직 거리
	const FVector Dir    = DashVelocity.GetSafeNormal();
	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	const FVector Lateral  = ToTarget - Dir * FVector::DotProduct(ToTarget, Dir);
	return Lateral.Size() <= PunchFrontHalfWidth;
}

void AMikaCharacter::ReboundFromHit()
{
	// 타격감: 멈춘 뒤 대시 반대 방향으로 튕긴다. 초기 속도 = 대시 거리 × 비율 ÷ 반동 시간 (충전 비례)
	const FVector Back = -DashVelocity.GetSafeNormal();
	EndDash();

	const float ReboundDistance = DashDistance * PunchReboundDistanceRatio;
	if (ReboundDistance <= KINDA_SMALL_NUMBER) return;

	PendingReboundVelocity = Back * (ReboundDistance / FMath::Max(PunchReboundTime, 0.01f));
	bIsRebounding = true;   // 딜레이 동안 제자리 (비행 모드라 떨어지지도 않음)

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->BrakingFrictionFactor     = 0.f;
	Move->BrakingDecelerationFlying = 0.f;
	Move->SetMovementMode(MOVE_Flying);
	Move->Velocity = FVector::ZeroVector;

	// 타격감: 부딪힌 자리에 잠깐 멈췄다가 튕긴다 (풀 충전 / 일반 따로)
	const float Delay = bDashFullCharge ? PunchReboundDelayFull : PunchReboundDelayNormal;
	if (Delay > KINDA_SMALL_NUMBER)
	{
		GetWorldTimerManager().SetTimer(ReboundDelayTimerHandle, this, &AMikaCharacter::BeginReboundMove, Delay, false);
	}
	else
	{
		BeginReboundMove();
	}
}

void AMikaCharacter::BeginReboundMove()
{
	// 초기 속도만 주고 끊지 않는다 — 지상은 걷기 제동, 공중은 관성·중력으로 자연스럽게 줄어듦
	bIsRebounding = false;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->BrakingFrictionFactor     = DefaultBrakingFrictionFactor;
	Move->BrakingDecelerationFlying = DefaultBrakingDecelerationFly;
	Move->SetMovementMode(MOVE_Falling);
	Move->Velocity = PendingReboundVelocity;
}

void AMikaCharacter::PunchSlam(const FVector& ImpactPoint)
{
	PlayPunchHitFX(ImpactPoint);
	if (bDebugDrawPunchHit)
	{
		DrawDebugSphere(GetWorld(), ImpactPoint, PunchSlamRadius, 24, FColor::Orange, false, 1.f);
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectTypes;
	ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
	ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);
	ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectTypes.AddObjectTypesToQuery(ECC_WorldStatic);   // 정적 파괴물
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PunchSlam), false, this);
	GetWorld()->OverlapMultiByObjectType(Overlaps, ImpactPoint, FQuat::Identity, ObjectTypes,
		FCollisionShape::MakeSphere(PunchSlamRadius), Params);

	TSet<AActor*> Hit;
	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* Actor = Result.GetActor();
		if (!Actor || Actor->GetOwner() == this || Hit.Contains(Actor)) continue;
		Hit.Add(Actor);
		// 착지 지점 기준 방사형, 정면과 같은 100% 피해·넉백
		FVector Dir = (Actor->GetActorLocation() - ImpactPoint).GetSafeNormal2D();
		if (Dir.IsNearlyZero()) Dir = DashVelocity.GetSafeNormal2D();
		ApplyPunchHit(Actor, Result.GetComponent(), Dir, 1.f, 1.f);
	}

	EndDash();
}

// --- 범위 데칼 ---

AMikaCharacter::EPunchPathEnd AMikaCharacter::TracePunchPath(float ChargeRatio, FVector& OutEnd, FVector& OutDir, FVector& OutNormal) const
{
	const FVector Start = GetActorLocation();
	OutDir    = GetPunchAimDirection();
	OutNormal = FVector::UpVector;
	const bool bSteepDown = OutDir.Rotation().Pitch <= -PunchSlamMinDownPitch;
	const bool bGrounded  = GetCharacterMovement()->IsMovingOnGround();

	// 지상에서 얕게 아래를 보면 수평 대시 (착지 공격도, 바닥 원도 없음)
	if (!bSteepDown && OutDir.Z < 0.f && bGrounded)
	{
		OutDir = OutDir.GetSafeNormal2D();
	}
	OutEnd = Start + OutDir * FMath::Lerp(PunchMinDistance, PunchMaxDistance, ChargeRatio);

	FHitResult Block;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PunchPath), false, this);

	if (bSteepDown && bGrounded)
	{
		// 지상에서 가파르게 아래: 조준선이 닿는 바닥까지 수평으로 달려가 그 지점에서 착지 공격
		// (캡슐로 쓸면 발밑 바닥에 바로 걸리므로 선으로)
		if (!GetWorld()->LineTraceSingleByChannel(Block, Start, OutEnd, ECC_Visibility, Params)) return EPunchPathEnd::Air;
		OutEnd = Block.ImpactPoint;
		// 적 머리 위 같은 곳은 바닥이 아니다
		const bool bFloor = !Cast<APawn>(Block.GetActor()) && GetCharacterMovement()->IsWalkable(Block);
		if (!bFloor && !Cast<APawn>(Block.GetActor())) OutNormal = Block.ImpactNormal;
		return bFloor ? EPunchPathEnd::Floor : EPunchPathEnd::Blocked;
	}

	// 그 외는 미카 캡슐 모양으로 쓸어본다 — 실제로 몸이 닿는 지점 = 실제 폭발 지점.
	// (선으로 예측하면 공중에서 아래로 칠 때 몸 중심선과 발이 닿는 곳이 1m 넘게 어긋나고, 벽 모서리도 놓친다)
	// 바닥을 스치지 않게 반높이를 조금 줄인다 (이동 컴포넌트와 같은 충돌 채널·응답 사용)
	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	FCollisionResponseParams Response;
	Capsule->InitSweepCollisionParams(Params, Response);
	const FCollisionShape Shape = FCollisionShape::MakeCapsule(
		Capsule->GetScaledCapsuleRadius() - 1.f, Capsule->GetScaledCapsuleHalfHeight() - 10.f);
	if (!GetWorld()->SweepSingleByChannel(Block, Start, OutEnd, FQuat::Identity,
		Capsule->GetCollisionObjectType(), Shape, Params, Response))
	{
		return EPunchPathEnd::Air;
	}

	const bool bPawn = Cast<APawn>(Block.GetActor()) != nullptr;
	if (GetCharacterMovement()->IsWalkable(Block) && !bPawn)
	{
		// 가파르게 아래면 발이 닿는 바닥이 착지 공격 지점, 얕으면 미끄러짐 (폭발 없음)
		if (!bSteepDown) return EPunchPathEnd::Air;
		OutEnd = Block.ImpactPoint;
		return EPunchPathEnd::Floor;
	}

	OutEnd = Block.ImpactPoint;
	if (!bPawn) OutNormal = Block.ImpactNormal;   // 벽이면 벽면에 투사, 적이면 발밑 바닥에 투사
	return EPunchPathEnd::Blocked;
}

void AMikaCharacter::UpdatePunchRangeDecal(float ChargeRatio)
{
	if (!PunchRangeDecalMaterial && !PunchExplosionDecalMaterial) return;

	// 지금 떼면 갈 경로: 조준 방향으로 충전 비례 거리. 벽·바닥에 막히면 거기까지 (대시와 같은 트레이스)
	const FVector Start = GetActorLocation();
	FVector End, PathDir, EndNormal;
	const EPunchPathEnd EndType = TracePunchPath(ChargeRatio, End, PathDir, EndNormal);

	// 폭발 범위: 실제로 폭발이 나는 경로 끝(착지 공격 지점 또는 벽)에만 폭발 반경 원. 허공에서 끝나면 숨김
	const float ExplosionRadius = GetPunchExplosionRadius(ChargeRatio);   // 충전할수록 커지는 원
	if (EndType == EPunchPathEnd::Air || ExplosionRadius <= 0.f)
	{
		PunchExplosionDecal->SetHiddenInGame(true);
	}
	else if (PunchExplosionDecalMaterial)
	{
		PunchExplosionDecal->SetHiddenInGame(false);
		// 원을 닿는 면에 수직으로 투사 (바닥 = 위에서 아래, 벽 = 벽면 정면). 중심은 실제 폭발 지점
		PunchExplosionDecal->SetWorldLocation(End);
		PunchExplosionDecal->SetWorldRotation((-EndNormal).Rotation());
		PunchExplosionDecal->DecalSize = FVector(GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 150.f,
			ExplosionRadius, ExplosionRadius);
		PunchExplosionDecal->MarkRenderStateDirty();
	}

	if (!PunchRangeDecalMaterial) return;

	// 바닥에 수평으로 투사. 바로 위·아래를 보면 길이가 없어 숨긴다
	FVector Flat = End - Start;
	Flat.Z = 0.f;
	const float Length = Flat.Size();
	if (Length < 1.f)
	{
		PunchRangeDecal->SetHiddenInGame(true);
		return;
	}
	PunchRangeDecal->SetHiddenInGame(false);

	// 데칼은 로컬 X로 투사 → Pitch -90으로 아래를 향하게 하면 로컬 Z가 진행 방향, Y가 폭
	// 투사 깊이는 발밑(캡슐 반높이 아래)과 아래로 조준한 끝점 높이까지 모두 덮도록
	const float DepthHalf = FMath::Abs(Start.Z - End.Z) * 0.5f + GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 100.f;
	PunchRangeDecal->SetWorldLocation((Start + End) * 0.5f);
	PunchRangeDecal->SetWorldRotation(FRotator(-90.f, Flat.Rotation().Yaw, 0.f));
	PunchRangeDecal->DecalSize = FVector(DepthHalf, PunchHitRadius, Length * 0.5f);
	PunchRangeDecal->MarkRenderStateDirty();
}

// --- 손 FX ---

float AMikaCharacter::GetChargeRatio() const
{
	const float HeldTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	return FMath::Clamp((HeldTime - MinChargeTime) / FMath::Max(MaxChargeTime - MinChargeTime, KINDA_SMALL_NUMBER), 0.f, 1.f);
}

void AMikaCharacter::StartHandVFX(UNiagaraSystem* VFX)
{
	StopHandVFX();
	if (!VFX) return;
	// 오른손 소켓 (ValveBiped에 hand_r 소켓 없음)
	ActiveHandVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
		VFX, GetMesh(), TEXT("WeaponSocketRight"),
		FVector::ZeroVector, FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget, true);
}

void AMikaCharacter::SetChargeVFXScale(float ChargeRatio)
{
	if (!ActiveHandVFX) return;
	const float Scale = FMath::Lerp(ChargeHandVFXMinScale, ChargeHandVFXMaxScale, ChargeRatio);
	if (HasScaleOverallParam(ChargeHandVFX))
	{
		ActiveHandVFX->SetVariableFloat(HandVFXScaleParam, Scale);
	}
	else
	{
		ActiveHandVFX->SetWorldScale3D(FVector(Scale));
	}
}

void AMikaCharacter::UpdateChargeVFXCameraOffset()
{
	if (!ActiveHandVFX || ChargeHandVFXCameraOffset <= 0.f) return;
	// 캐릭터는 불투명이라 손이 몸 뒤로 가면 반투명 VFX가 가려진다 — 카메라 쪽으로 당겨 몸보다 앞에 둔다
	const FVector HandLoc = GetMesh()->GetSocketLocation(TEXT("WeaponSocketRight"));
	const FVector ToCamera = (CameraComponent->GetComponentLocation() - HandLoc).GetSafeNormal();
	ActiveHandVFX->SetWorldLocation(HandLoc + ToCamera * ChargeHandVFXCameraOffset);
}

void AMikaCharacter::StopFullChargeVFX()
{
	if (ActiveFullChargeVFX) ActiveFullChargeVFX->Deactivate();
	ActiveFullChargeVFX = nullptr;
}

void AMikaCharacter::StopHandVFX()
{
	// Deactivate: 새 파티클만 멈추고 남은 파티클은 자연 소멸 (자동 파괴)
	if (ActiveHandVFX) ActiveHandVFX->Deactivate();
	ActiveHandVFX = nullptr;
}

void AMikaCharacter::PlayHandSFX(USoundBase* SFX)
{
	StopHandSFX();
	if (SFX) ActiveHandSFX = UGameplayStatics::SpawnSoundAttached(SFX, GetMesh(), TEXT("WeaponSocketRight"));
}

void AMikaCharacter::StopHandSFX()
{
	if (ActiveHandSFX) ActiveHandSFX->Stop();
	ActiveHandSFX = nullptr;
}

void AMikaCharacter::PlayPunchHitFX(const FVector& Location)
{
	// 진단용 (히트 FX가 안 보이던 원인 확인 후 제거)
	UE_LOG(LogTemp, Log, TEXT("[PunchHitFX] at %s, already played=%d, VFX=%s, SFX=%s"),
		*Location.ToString(), bDashHitFXPlayed ? 1 : 0, *GetNameSafe(PunchHitVFX), *GetNameSafe(PunchHitSFX));

	if (bDashHitFXPlayed) return;
	bDashHitFXPlayed = true;

	// 폭발은 충전 비례 크기로 항상
	PunchExplode(Location);

	// 풀 충전이 아니면 메인 연출 없이 서브 히트 FX만 (대미지·멈춤·반동은 그대로)
	if (!bDashFullCharge)
	{
		PlayPunchSideHitFX(Location);
		return;
	}

	// 충전할수록 크고 큰 소리 (풀 충전만 오므로 사실상 Max 쪽. Min은 MaxChargeTime을 바꿀 때 대비)
	SpawnHitFX(PunchHitVFX, FMath::Lerp(PunchHitVFXMinScale, PunchHitVFXMaxScale, DashChargeRatio),
		PunchHitSFX, FMath::Lerp(PunchHitSFXMinVolume, PunchHitSFXMaxVolume, DashChargeRatio), Location);
	StartHitSlow();
}

void AMikaCharacter::PunchExplode(const FVector& Location)
{
	const float ExplosionRadius = GetPunchExplosionRadius(DashChargeRatio);
	if (PunchExplosionVFX)
	{
		const float Scale = ExplosionRadius / FMath::Max(PunchExplosionVFXReferenceRadius, 1.f);
		UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), PunchExplosionVFX, Location, FRotator::ZeroRotator, FVector(1.f));
		if (Comp && HasScaleOverallParam(PunchExplosionVFX))
		{
			Comp->SetVariableFloat(HandVFXScaleParam, Scale);
		}
		else if (Comp)
		{
			Comp->SetWorldScale3D(FVector(Scale));
		}
	}

	if (ExplosionRadius > 0.f)
	{
		// 반경 안 전원(미카·미카 소유 제외)에 충전 비례 폭발 피해 + 폭발 중심에서 바깥쪽 넉백
		const float Damage = FMath::Lerp(PunchExplosionMinDamage, PunchExplosionMaxDamage, DashChargeRatio);
		const float KnockbackSpeed = DashDistance * PunchKnockbackPerDistance * PunchExplosionKnockbackRatio;

		TArray<FOverlapResult> Overlaps;
		FCollisionObjectQueryParams ObjectTypes;
		ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
		ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);
		ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(PunchExplosion), false, this);
		GetWorld()->OverlapMultiByObjectType(Overlaps, Location, FQuat::Identity, ObjectTypes,
			FCollisionShape::MakeSphere(ExplosionRadius), Params);

		TSet<AActor*> Done;
		for (const FOverlapResult& Result : Overlaps)
		{
			AActor* Actor = Result.GetActor();
			if (!Actor || Actor->GetOwner() == this || Done.Contains(Actor)) continue;
			Done.Add(Actor);

			if (Damage > 0.f)
			{
				UGameplayStatics::ApplyDamage(Actor, Damage, GetController(), this, UDamageType::StaticClass());
			}
			if (KnockbackSpeed > 0.f)
			{
				FVector Dir = (Actor->GetActorLocation() - Location).GetSafeNormal2D();
				if (Dir.IsNearlyZero()) Dir = DashVelocity.GetSafeNormal2D();
				const FVector Knockback = Dir * KnockbackSpeed + FVector::UpVector * KnockbackSpeed * PunchKnockbackUpRatio;
				if (ACharacter* HitCharacter = Cast<ACharacter>(Actor))
				{
					HitCharacter->LaunchCharacter(Knockback, true, true);
				}
				else if (UPrimitiveComponent* Comp = Result.GetComponent(); Comp && Comp->IsSimulatingPhysics())
				{
					Comp->AddImpulse(Knockback, NAME_None, true);
				}
			}
		}
	}
	if (bDebugDrawPunchHit)
	{
		DrawDebugSphere(GetWorld(), Location, ExplosionRadius, 24, FColor::Magenta, false, 1.f);
	}
}

void AMikaCharacter::SetPunchRangeDecalFilled(bool bFilled)
{
	if (PunchRangeDecalMID)
	{
		PunchRangeDecalMID->SetScalarParameterValue(TEXT("FillOpacity"), bFilled ? PunchRangeDecalFillOpacity : 0.f);
	}
	if (PunchExplosionDecalMID)
	{
		PunchExplosionDecalMID->SetScalarParameterValue(TEXT("FillOpacity"), bFilled ? PunchExplosionDecalFillOpacity : 0.f);
	}
}

void AMikaCharacter::PlayPunchSideHitFX(const FVector& Location)
{
	SpawnHitFX(PunchSideHitVFX, PunchSideHitVFXScale, PunchSideHitSFX, 1.f, Location);
}

void AMikaCharacter::StartHitSlow()
{
	if (PunchHitSlowDuration <= 0.f) return;
	const float TimeScale = FMath::Clamp(PunchHitSlowTimeScale, 0.01f, 1.f);
	UGameplayStatics::SetGlobalTimeDilation(this, TimeScale);
	// 타이머도 느려진 시간으로 흐르므로 × 배속 → 실제 시간으로 PunchHitSlowDuration 뒤 복구
	GetWorldTimerManager().SetTimer(HitSlowTimerHandle, this, &AMikaCharacter::EndHitSlow,
		PunchHitSlowDuration * TimeScale, false);
}

void AMikaCharacter::EndHitSlow()
{
	GetWorldTimerManager().ClearTimer(HitSlowTimerHandle);
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
}

void AMikaCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 히트스톱 도중 파괴·레벨 전환돼도 전역 배속이 느린 채로 남지 않게
	if (GetWorldTimerManager().IsTimerActive(HitSlowTimerHandle))
	{
		EndHitSlow();
	}
	Super::EndPlay(EndPlayReason);
}

void AMikaCharacter::SpawnHitFX(UNiagaraSystem* VFX, float Scale, USoundBase* SFX, float Volume, const FVector& Location)
{
	if (VFX)
	{
		// 총기 히트처럼 X축이 공격자(미카) 쪽을 향하게
		const FRotator FacingMika = (GetActorLocation() - Location).Rotation();
		UNiagaraComponent* HitComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), VFX, Location, FacingMika, FVector(1.f));
		if (HitComp && HasScaleOverallParam(VFX))
		{
			HitComp->SetVariableFloat(HandVFXScaleParam, Scale);
		}
		else if (HitComp)
		{
			HitComp->SetWorldScale3D(FVector(Scale));
		}
	}
	if (SFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SFX, Location, Volume);
	}
}

void AMikaCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	if (!bIsDashing) return;
	if (Cast<APawn>(Other)) return;   // 적은 판정 캡슐 오버랩이 처리

	if (GetCharacterMovement()->IsWalkable(Hit))
	{
		// 바닥 목표 대시(범위 원이 바닥에 떴던 대시)면 바닥에 닿는 순간 착지 공격. 아니면 바닥을 스치는 건 무시.
		// 이동 처리 도중이라 여기서 이동 모드를 바꾸지 않고 다음 Tick에서 실행
		if (bDashFloorTarget && !bPendingSlam)
		{
			bPendingSlam     = true;
			PendingSlamPoint = Hit.ImpactPoint;
		}
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[PunchHitFX] wall hit %s"), *GetNameSafe(Other));
	// 벽도 적 정면 적중처럼 멈추고 반동
	PlayPunchHitFX(Hit.ImpactPoint);
	bPendingRebound = true;
}

void AMikaCharacter::RestoreWeaponAfter(float Delay)
{
	if (Delay > KINDA_SMALL_NUMBER)
	{
		GetWorldTimerManager().SetTimer(WeaponRestoreTimerHandle, this, &AMikaCharacter::RestoreWeapon, Delay, false);
	}
	else
	{
		RestoreWeapon();
	}
}

// --- 판정 캡슐 오버랩 ---

void AMikaCharacter::OnPunchHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsDashing) return;
	if (!OtherActor || OtherActor == this) return;
	if (OtherActor->GetOwner() == this) return;   // 손에 든(숨긴) 총·수류탄
	if (HitActorsDuringDash.Contains(OtherActor)) return;

	HitActorsDuringDash.Add(OtherActor);
	UE_LOG(LogTemp, Log, TEXT("[PunchHitFX] overlap %s (%s)"), *GetNameSafe(OtherActor), *GetNameSafe(OtherComp));

	// 맞은 위치 = 미카에서 대상 중심으로 그 컴포넌트만 트레이스한 표면 지점. 실패하면 판정 캡슐 위치
	FVector HitPoint = PunchHitCapsule->GetComponentLocation();
	if (OtherComp)
	{
		FHitResult SurfaceHit;
		if (OtherComp->LineTraceComponent(SurfaceHit, GetActorLocation(), OtherComp->Bounds.Origin,
			FCollisionQueryParams(SCENE_QUERY_STAT(PunchHitFX), true)))
		{
			HitPoint = SurfaceHit.ImpactPoint;
		}
	}

	// 파괴물은 정면·측면 구분 없이 부수고 계속 진행
	if (Cast<IDestructibleObject>(OtherActor))
	{
		PlayPunchHitFX(HitPoint);
		ApplyPunchHit(OtherActor, OtherComp, FVector::ZeroVector, 1.f, 1.f);
		return;
	}

	if (IsFrontHit(OtherActor))
	{
		// 정면: 100% 피해, 대상은 미카 정면(대시 수평 방향)으로, 미카는 멈추고 반동
		PlayPunchHitFX(HitPoint);
		ApplyPunchHit(OtherActor, OtherComp, DashVelocity.GetSafeNormal2D(), 1.f, 1.f);
		if (Cast<APawn>(OtherActor)) bPendingRebound = true;
	}
	else
	{
		// 측면: 비율 피해, 대상은 미카 → 대상 방향(피격 위치 쪽)으로, 미카는 계속 진행
		FVector Dir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		if (Dir.IsNearlyZero()) Dir = DashVelocity.GetSafeNormal2D();
		PlayPunchSideHitFX(HitPoint);
		ApplyPunchHit(OtherActor, OtherComp, Dir, PunchSideDamageRatio, PunchSideKnockbackRatio);
	}
}
