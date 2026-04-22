// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "MikaDataAsset.h"
#include "MikaCharacter.generated.h"

class UBoxComponent;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AMikaCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	AMikaCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Data ---

	/** 할당 시 BeginPlay에서 모든 스탯을 덮어씀 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Data")
	UMikaDataAsset* MikaData = nullptr;

	// --- Punch / Landing 공유 ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mika|Punch")
	UBoxComponent* PunchHitbox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float PunchDamage = 30.0f;

	/** 펀치·랜딩 공유 쿨타임 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float PunchCooldown = 1.0f;

	/** 충전 중 중력 배율 (0 = 무중력, 1 = 기본) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float ChargeGravityScale = 0.2f;

	/** 충전 중 이동속도 배율 (0 = 완전 정지, 1 = 기본) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float ChargeMovementScale = 0.3f;

	/** 충전 중 지상 감속도 (클수록 빠르게 제동, 기본 UE값 = 2048) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float ChargeBrakingDeceleration = 600.f;

	/** 충전 중 공중 감속도 (클수록 공중 관성이 빠르게 줄어듦, 기본 UE값 = 0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float ChargeAirBrakingDeceleration = 1000.f;

	/** 최소 충전 시간 (초). 이 미만이면 미발동 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float MinChargeTime = 0.1f;

	/** 최대 충전 시간 (초). 이 이상 충전해도 속도가 더 오르지 않음 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float MaxChargeTime = 1.5f;

	/** 강제 발동 시간 (초). 이 이상 충전하면 자동으로 발동 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float ForcedMaxChargeTime = 2.0f;

	/** 최소 충전 시 대시 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float MinDashSpeed = 800.0f;

	/** 최대 충전 시 대시 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float MaxDashSpeed = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float DashDuration = 0.25f;

	/** 대시 중 공중 제동력 (클수록 짧게 날아감, 기본 UE = 0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch")
	float DashBrakingDeceleration = 800.f;

	// --- Punch Camera ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera")
	float ChargeSpringArmLength = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera")
	float ChargeFOV = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera")
	float DashSpringArmLength = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera")
	float DashFOV = 105.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera")
	float CameraInterpSpeed = 10.0f;

	// --- Landing ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Landing")
	float LandingDamage = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Landing")
	float LandingRadius = 300.0f;

	/** 아래를 바라보는 최소 피치 각도 (예: 60 → 카메라가 -60° 이하여야 함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Landing")
	float LandingDownAngle = 60.0f;

	/** 랜딩 발동을 위한 지면으로부터 최소 높이 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Landing")
	float LandingMinHeight = 300.0f;

	/** 아래로 다이브하는 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Landing")
	float LandingDiveSpeed = 2000.0f;

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State")
	bool bCanPunch        = true;

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State")
	bool bIsChargingPunch = false;

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State")
	bool bIsDashing       = false;

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State")
	bool bIsDivingLanding = false;

	float ChargeStartTime            = 0.f;
	float NormalSpringArmLength      = 300.f;
	float DefaultBrakingDeceleration     = 2048.f;
	float DefaultBrakingDecelerationFly  = 0.f;
	float DefaultBrakingDecelerationFall = 0.f;

	TSet<AActor*> HitActorsDuringDash;

	FTimerHandle DashEndTimerHandle;
	FTimerHandle PunchCooldownTimerHandle;
	FTimerHandle AutoReleaseTimerHandle;

	// --- Overrides ---

	virtual void StartFire() override;
	virtual void StopFire() override;
	virtual void Landed(const FHitResult& Hit) override;

	// --- Internal ---

	void StartDash(float ChargeRatio);
	void EndDash();
	void StartPunchCooldown();
	void ResetPunchCooldown();
	bool CanTriggerLanding() const;

	UFUNCTION()
	void OnPunchHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

public:
	UFUNCTION(BlueprintCallable, Category = "Mika|Combat")
	void MikaLanding();
};
