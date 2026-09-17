// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "MikaDataAsset.h"
#include "MikaCharacter.generated.h"

class UBoxComponent;
class UHealthRegenComponent;

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

	// --- Health Regen ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mika|Regen", meta=(ToolTip="비전투 체력 회복. 수치는 MikaData의 Stats|Regen"))
	UHealthRegenComponent* HealthRegen;

public:
	// 체력바 WBP가 OnRegenStarted/Stopped를 구독할 때 사용
	UFUNCTION(BlueprintPure, Category = "Mika|Regen")
	UHealthRegenComponent* GetHealthRegen() const { return HealthRegen; }

protected:
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="true면 대시 지속 시간 = 펀치 몽타주 재생 길이. MikaData에서 설정"))
	bool bDashDurationFromMontage = true;

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

	// --- Punch Animation ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Animation", meta=(ToolTip="펀치 충전 몽타주. MikaData에서 설정"))
	UAnimMontage* PunchChargeMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Animation", meta=(ToolTip="대시 펀치 몽타주. MikaData에서 설정"))
	UAnimMontage* PunchDashMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Animation", meta=(ToolTip="랜딩 다이브 낙하 몽타주. MikaData에서 설정"))
	UAnimMontage* LandingDiveMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Animation", meta=(ToolTip="랜딩 착지 공격 몽타주. MikaData에서 설정"))
	UAnimMontage* LandingImpactMontage = nullptr;

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
	FTimerHandle WeaponRestoreTimerHandle;   // 펀치·착지 몽타주가 끝난 뒤 총 복구

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

	// 맨손 공격 동안 총을 숨긴다 — 펀치 애니가 총을 쥔 손과 겹쳐 보이지 않도록. 충전 시작에 숨기고 공격 종료 시 복구
	void SetWeaponHiddenForPunch(bool bHideWeapon);

	// 몽타주 길이만큼 기다렸다 총 복구 — 대시(0.25초)보다 펀치 애니가 길어 대시 종료 시점에 복구하면 총이 거의 안 숨는다
	void RestoreWeaponAfter(float Delay);
	void RestoreWeapon() { SetWeaponHiddenForPunch(false); }
	float PunchMontageEndTime = 0.f;

	// 몽타주를 재생하고 실제 재생 시간(초)을 돌려준다. PlayAnimMontage는 Rate Scale을 반영하지 않은 원본 길이를 돌려주므로 보정
	float PlayMontageForDuration(UAnimMontage* Montage);

	UFUNCTION()
	void OnPunchHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

public:
	UFUNCTION(BlueprintCallable, Category = "Mika|Combat")
	void MikaLanding();
};
