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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="미카의 최대 체력"))
	float MaxHealth = 100.f;

	// --- Movement ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta=(ToolTip="걷기 속도 (cm/s)"))
	float WalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta=(ToolTip="달리기 속도 (cm/s)"))
	float SprintSpeed = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta=(ToolTip="조준 중 걷기 속도 (cm/s)"))
	float AimWalkSpeed = 250.f;

	// --- Camera ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ToolTip="기본 카메라 시야각 (도)"))
	float NormalFOV = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ToolTip="조준 시 카메라 시야각 (도). 낮을수록 줌인"))
	float AimFOV = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ToolTip="좌우 엄폐물 감지 거리 (cm)"))
	float CoverTraceDistance = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ToolTip="엄폐 시 카메라가 옆으로 이동하는 거리 (cm)"))
	float CoverPeekOffset = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ToolTip="엄폐 카메라 이동 보간 속도. 클수록 빠르게 전환"))
	float CoverPeekInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ToolTip="척추 피치 클램프 범위 (±도). 상하 조준 제한각"))
	float AimSpinePitchClamp = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ToolTip="척추 피치 보간 속도. 클수록 상하 조준이 빠르게 반응"))
	float AimSpineInterpSpeed = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ToolTip="기본 카메라 소켓 좌우 오프셋 (양수=오른쪽, cm)"))
	float DefaultSocketOffsetY = 60.f;

	// --- TurnInPlace ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurnInPlace", meta=(ToolTip="제자리 우회전 발동 각도 (도). 카메라가 이 각도 이상 오른쪽으로 벌어지면 회전 시작"))
	float TurnRightThreshold = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurnInPlace", meta=(ToolTip="제자리 좌회전 발동 각도 (도)"))
	float TurnLeftThreshold = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurnInPlace", meta=(ToolTip="제자리 회전 중 액터 회전 속도 (도/초)"))
	float TurnRotationSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurnInPlace", meta=(ToolTip="임계값 미만일 때 카메라를 부드럽게 따라가는 속도 (도/초). 0이면 비활성화"))
	float SoftTurnSpeed = 60.f;

	// --- Fall ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall", meta=(ToolTip="하드 랜딩 판정 최소 낙하 속도 (cm/s). 이 이상 속도로 착지하면 경착지 애니메이션 재생"))
	float HardLandingSpeedThreshold = 600.f;

	// --- Grenade ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="수류탄 초기 투사 속도 (cm/s)"))
	float GrenadeThrowSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="최대 수류탄 보유 개수"))
	int32 MaxGrenadeCount = 4;

	// --- Weapon ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta=(ToolTip="무기 교체 딜레이 (초). 이 시간 후 새 무기가 장착됨"))
	float WeaponSwapDelay = 0.3f;

	// --- Punch ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="펀치 대시 히트 시 피해량"))
	float PunchDamage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="펀치·랜딩 공유 쿨타임 (초)"))
	float PunchCooldown = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="충전 중 중력 배율 (0=무중력, 1=기본). 낮을수록 공중에 떠 있는 느낌"))
	float ChargeGravityScale = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="충전 중 이동속도 배율 (0=완전 정지, 1=기본 속도)"))
	float ChargeMovementScale = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="충전 중 지상 감속도. 클수록 빠르게 제동 (UE 기본값 2048)"))
	float ChargeBrakingDeceleration = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="충전 중 공중 감속도. 클수록 공중 관성이 빠르게 줄어듦 (UE 기본값 0)"))
	float ChargeAirBrakingDeceleration = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="최소 충전 시간 (초). 이 미만으로 눌렀다 떼면 미발동"))
	float MinChargeTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="최대 충전 시간 (초). 이 이상 충전해도 대시 속도가 더 오르지 않음"))
	float MaxChargeTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="강제 발동 시간 (초). 이 이상 충전하면 자동으로 발동됨"))
	float ForcedMaxChargeTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="최소 충전(MinChargeTime) 시의 대시 속도 (cm/s)"))
	float MinDashSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="최대 충전(MaxChargeTime) 시의 대시 속도 (cm/s)"))
	float MaxDashSpeed = 2400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="대시 지속 시간 (초). 히트박스가 활성화되는 시간"))
	float DashDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="대시 중 공중 감속도. 클수록 짧게 날아감"))
	float DashBrakingDeceleration = 800.f;

	// --- Punch Camera ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera", meta=(ToolTip="충전 중 스프링암 길이 (cm). 짧을수록 카메라가 캐릭터에 가까워짐"))
	float ChargeSpringArmLength = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera", meta=(ToolTip="충전 중 시야각 (도)"))
	float ChargeFOV = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera", meta=(ToolTip="대시 중 스프링암 길이 (cm). 길수록 넓은 화면"))
	float DashSpringArmLength = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera", meta=(ToolTip="대시 중 시야각 (도). 클수록 속도감 연출"))
	float DashFOV = 105.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera", meta=(ToolTip="카메라 스프링암·FOV 보간 속도. 클수록 빠르게 전환"))
	float CameraInterpSpeed = 10.f;

	// --- Landing ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing", meta=(ToolTip="랜딩 착지 시 범위 피해량"))
	float LandingDamage = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing", meta=(ToolTip="랜딩 피해 반경 (cm)"))
	float LandingRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing", meta=(ToolTip="랜딩 발동을 위한 최소 카메라 하향 각도 (도). 60이면 카메라가 -60도 이하여야 발동"))
	float LandingDownAngle = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing", meta=(ToolTip="랜딩 발동 최소 지면까지의 거리 (cm). 이 이상 높아야 발동 가능"))
	float LandingMinHeight = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing", meta=(ToolTip="랜딩 다이브 낙하 속도 (cm/s). 클수록 빠르게 내려꽂힘"))
	float LandingDiveSpeed = 2000.f;
};
