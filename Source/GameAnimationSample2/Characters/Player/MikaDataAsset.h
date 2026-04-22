// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MikaDataAsset.generated.h"

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UMikaDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 100.f;

	// --- Movement ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float AimWalkSpeed = 250.f;

	// --- Camera ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float NormalFOV = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float AimFOV = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CoverTraceDistance = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CoverPeekOffset = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CoverPeekInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float DefaultSocketOffsetY = 60.f;

	// --- TurnInPlace ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurnInPlace")
	float TurnRightThreshold = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurnInPlace")
	float TurnLeftThreshold = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurnInPlace")
	float TurnRotationSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurnInPlace")
	float SoftTurnSpeed = 60.f;

	// --- Fall ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall")
	float HardLandingSpeedThreshold = 600.f;

	// --- Grenade ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grenade")
	float GrenadeThrowSpeed = 1200.f;

	// --- Weapon ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float WeaponSwapDelay = 0.3f;

	// --- Punch ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float PunchDamage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float PunchCooldown = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float ChargeGravityScale = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float ChargeMovementScale = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float ChargeBrakingDeceleration = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float ChargeAirBrakingDeceleration = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float MinChargeTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float MaxChargeTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float ForcedMaxChargeTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float MinDashSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float MaxDashSpeed = 2400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float DashDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch")
	float DashBrakingDeceleration = 800.f;

	// --- Punch Camera ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera")
	float ChargeSpringArmLength = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera")
	float ChargeFOV = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera")
	float DashSpringArmLength = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera")
	float DashFOV = 105.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera")
	float CameraInterpSpeed = 10.f;

	// --- Landing ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	float LandingDamage = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	float LandingRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	float LandingDownAngle = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	float LandingMinHeight = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	float LandingDiveSpeed = 2000.f;
};
