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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Data", meta=(ToolTip="할당 시 BeginPlay에서 모든 스탯을 덮어씀"))
	UMikaDataAsset* MikaData = nullptr;

	// --- Punch ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mika|Punch", meta=(ToolTip="펀치 히트박스 컴포넌트"))
	UBoxComponent* PunchHitbox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="펀치 피해량"))
	float PunchDamage = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="펀치·랜딩 공유 쿨타임 (초)"))
	float PunchCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="충전 중 중력 배율 (0=무중력, 1=기본). 낮을수록 공중에 떠 있는 느낌"))
	float ChargeGravityScale = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="충전 중 이동속도 배율 (0=완전 정지, 1=기본 속도)"))
	float ChargeMovementScale = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="충전 중 지상 감속도. 클수록 빠르게 제동 (UE 기본값 2048)"))
	float ChargeBrakingDeceleration = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="충전 중 공중 감속도. 클수록 공중 관성이 빠르게 줄어듦 (UE 기본값 0)"))
	float ChargeAirBrakingDeceleration = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="최소 충전 시간 (초). 이 미만으로 눌렀다 떼면 미발동"))
	float MinChargeTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="최대 충전 시간 (초). 이 이상 충전해도 대시 속도가 더 오르지 않음"))
	float MaxChargeTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="강제 발동 시간 (초). 이 이상 충전하면 자동으로 발동됨"))
	float ForcedMaxChargeTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="최소 충전(MinChargeTime) 시의 대시 속도 (cm/s)"))
	float MinDashSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="최대 충전(MaxChargeTime) 시의 대시 속도 (cm/s)"))
	float MaxDashSpeed = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="대시 지속 시간 (초). 히트박스가 활성화되는 시간"))
	float DashDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="대시 중 공중 제동력. 클수록 짧게 날아감 (UE 기본값 0)"))
	float DashBrakingDeceleration = 800.f;

	// --- Punch Camera ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera", meta=(ToolTip="충전 중 스프링암 길이 (cm). 짧을수록 카메라가 캐릭터에 가까워짐"))
	float ChargeSpringArmLength = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera", meta=(ToolTip="충전 중 시야각 (도)"))
	float ChargeFOV = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera", meta=(ToolTip="대시 중 스프링암 길이 (cm). 길수록 넓은 화면"))
	float DashSpringArmLength = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera", meta=(ToolTip="대시 중 시야각 (도). 클수록 속도감 연출"))
	float DashFOV = 105.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera", meta=(ToolTip="카메라 스프링암·FOV 보간 속도. 클수록 빠르게 전환"))
	float CameraInterpSpeed = 10.0f;

	// --- Landing ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Landing", meta=(ToolTip="랜딩 착지 시 범위 피해량"))
	float LandingDamage = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Landing", meta=(ToolTip="랜딩 피해 반경 (cm)"))
	float LandingRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Landing", meta=(ToolTip="랜딩 발동을 위한 최소 카메라 하향 각도 (도). 60이면 카메라가 -60도 이하여야 발동"))
	float LandingDownAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Landing", meta=(ToolTip="랜딩 발동 최소 지면까지의 거리 (cm). 이 이상 높아야 발동 가능"))
	float LandingMinHeight = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Landing", meta=(ToolTip="랜딩 다이브 낙하 속도 (cm/s). 클수록 빠르게 내려꽂힘"))
	float LandingDiveSpeed = 2000.0f;

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State", meta=(ToolTip="펀치 사용 가능 여부. 쿨타임 중이면 false"))
	bool bCanPunch        = true;

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State", meta=(ToolTip="현재 펀치 충전 중인지"))
	bool bIsChargingPunch = false;

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State", meta=(ToolTip="현재 대시 중인지"))
	bool bIsDashing       = false;

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State", meta=(ToolTip="현재 랜딩 다이브 중인지"))
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
