// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MikaDataAsset.generated.h"

class UAnimMontage;
class AGrenadeBase;
class UNiagaraSystem;
class USoundBase;
class UMaterialInterface;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UMikaDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="미카의 최대 체력"))
	float MaxHealth = 100.f;

	// --- Health Regen ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Regen", meta=(ClampMin="0", ToolTip="마지막 피격 후 회복 시작까지 대기 시간 (초)"))
	float RegenDelay = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Regen", meta=(ClampMin="0.01", ToolTip="회복 간격 N (초). N초마다 RegenAmount만큼 회복"))
	float RegenInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Regen", meta=(ClampMin="0", ToolTip="간격마다 회복량 M (체력 수치)"))
	float RegenAmount = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Regen", meta=(ClampMin="0", ClampMax="1", ToolTip="회복 상한 (최대 체력 대비 비율). 0.5면 50%까지만 차오르고, 그 위에서 맞으면 회복 없음"))
	float RegenCapRatio = 0.5f;

	// --- Movement ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta=(ToolTip="걷기 속도 (cm/s)"))
	float WalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta=(ToolTip="달리기 속도 (cm/s)"))
	float SprintSpeed = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta=(ToolTip="조준 중 걷기 속도 (cm/s)"))
	float AimWalkSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta=(ToolTip="앉아서 이동하는 속도 (cm/s)"))
	float CrouchWalkSpeed = 200.f;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ClampMin="-89.9", ClampMax="0", ToolTip="카메라가 내려다볼 수 있는 최대 각도 (음수, 도). 엔진 기본 -89.9"))
	float CameraPitchMin = -89.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ClampMin="0", ClampMax="89.9", ToolTip="카메라가 올려다볼 수 있는 최대 각도 (도). 엔진 기본 89.9"))
	float CameraPitchMax = 89.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ClampMin="-89.9", ClampMax="0", ToolTip="펀치 충전 중에만 쓰는 카메라 최저 피치(도). 바닥을 내려찍으려면 평소보다 더 아래를 봐야 하므로 따로 둔다. 충전이 끝나면 CameraPitchMin으로 복귀"))
	float ChargeCameraPitchMin = -89.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ToolTip="척추 피치 보간 속도. 클수록 상하 조준이 빠르게 반응"))
	float AimSpineInterpSpeed = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Aim", meta=(ToolTip="조준 중 카메라를 기본 위치(DefaultSocketOffsetY)에서 더 오른쪽으로 옮기는 거리(cm, 음수=왼쪽). 보간 속도는 CoverPeekInterpSpeed"))
	float AimSocketOffsetRight = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Aim", meta=(ToolTip="조준 중 카메라를 위로 올리는 거리(cm, 음수=아래)"))
	float AimSocketOffsetUp = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|AimWaist", meta=(ClampMin="-90", ClampMax="90", ToolTip="조준 중 허리(Spine1)를 오른쪽으로 더 트는 각도(도, 음수면 왼쪽). ABP가 AimWaistYaw로 적용"))
	float AimWaistYawOffset = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|AimWaist", meta=(ClampMin="0", ToolTip="조준 시작·해제 때 허리 틀기 보간 속도. 클수록 빠르게 (0이면 즉시)"))
	float AimWaistBlendSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta=(ToolTip="기본 카메라 소켓 좌우 오프셋 (양수=오른쪽, cm)"))
	float DefaultSocketOffsetY = 60.f;

	// --- Camera (Crouch) ---
	// 앉으면 캡슐이 낮아져 시점이 내려가는 것을 스프링암 TargetOffset.Z로 보정한다.
	// TargetOffset.Z는 다른 전투 카메라 로직(FOV·암 길이·SocketOffset.Y)과 독립적이라 얽힘 없음.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Crouch", meta=(ToolTip="앉기 시 카메라를 위로 올리는 높이 (cm, 월드 기준)"))
	float CrouchCameraZOffset = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Crouch", meta=(ToolTip="true=앉기 카메라 높이를 보간(부드럽게), false=즉시 전환"))
	bool bSmoothCrouchCamera = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Crouch", meta=(ToolTip="앉기 카메라 보간 속도. bSmoothCrouchCamera=true일 때만 사용. 클수록 빠르게 전환"))
	float CrouchCameraInterpSpeed = 10.f;

	// --- Camera (메시 따라가기) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|FollowMesh", meta=(ToolTip="켜면 공중 포즈(점프·공중 펀치)에서 카메라가 보이는 몸을 따라 올라간다. 점프 모션은 골반이 캡슐보다 크게 올라가서 몸이 화면 밖으로 나가는데, 그걸 막는다"))
	bool bCameraFollowMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|FollowMesh", meta=(ToolTip="카메라가 따라갈 기준 본 (기본: 골반)"))
	FName CameraFollowMeshBone = TEXT("valvebiped_bip01_pelvis");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|FollowMesh", meta=(ClampMin="0", ToolTip="따라가기 보간 속도. 클수록 즉시 따라가고, 낮추면 부드럽게 (너무 크면 모션 흔들림이 카메라에 그대로 전달)"))
	float CameraFollowMeshInterpSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|FollowMesh", meta=(ClampMin="0", ToolTip="최대 보정 높이(cm). 모션이 크게 튀어도 이 이상은 안 따라간다"))
	float CameraFollowMeshMaxOffset = 200.f;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall", meta=(ClampMin="0", ToolTip="착지 후 공중 포즈(점프 상태 머신)를 유지하는 시간(초). 착지 모션 길이에 맞춘다. ABP는 bIsInAirPose를 읽음"))
	float LandPoseHoldTime = 0.47f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall", meta=(ClampMin="0", ToolTip="착지 몇 초 전에 착지 모션을 시작할지. ABP는 bIsLandingSoon을 Land 전환 조건으로 사용. 0이면 예측 끔"))
	float LandAnticipationTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall", meta=(ClampMin="0", ToolTip="공중 포즈 가중치(AirPoseAlpha) 보간 속도. ABP 하체 레이어 Blend Weights에 연결. 클수록 빠르게 전환"))
	float AirPoseBlendSpeed = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall", meta=(ClampMin="0.1", ToolTip="공중 모션(JumpStart·JumpUp·JumpDown·FallLoop) 재생 속도 배율. 1 = 원본, 0.8 = 20% 느리게. 착지는 LandAnimPlayRate로 따로 설정"))
	float JumpAnimPlayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall", meta=(ClampMin="0.1", ToolTip="착지(Land) 모션만의 재생 속도 배율. 착지 유지 시간도 이 값에 맞춰 자동 계산된다"))
	float LandAnimPlayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall", meta=(ToolTip="ABP Land 상태가 재생하는 착지 클립(AC_Land_mika). 지정하면 착지 유지 시간을 길이에서 자동 계산해 LandPoseHoldTime을 무시. 클립 길이를 바꿔도 코드 수정 불필요"))
	TObjectPtr<UAnimSequenceBase> LandAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall", meta=(ClampMin="0", ToolTip="착지 클립을 몇 초 지점부터 재생할지. ABP Land 상태 Start Position에 바인딩됨 (발 닿기 직전부터 재생)"))
	float LandAnimStartTime = 0.05f;

	// --- Grenade ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="던질 수류탄 액터 클래스 (BP_Grenade_Thrown). 투사체 수치·VFX·SFX는 그 BP의 GrenadeData(GrenadeData_Throwable)에서"))
	TSubclassOf<AGrenadeBase> GrenadeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="수류탄 초기 투사 속도 (cm/s)"))
	float GrenadeThrowSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="조준선 시작점 = 실제 발사 위치. 카메라 기준 오프셋 (X=앞, Y=오른쪽, Z=위, cm). 수류탄은 항상 이 조준선 그대로 날아간다"))
	FVector GrenadeLaunchOffset = FVector(50.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grenade", meta=(ToolTip="최대 수류탄 보유 개수"))
	int32 MaxGrenadeCount = 4;

	// --- Weapon ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta=(ToolTip="무기 교체 딜레이 (초). 이 시간 후 새 무기가 장착됨"))
	float WeaponSwapDelay = 0.3f;

	// --- Punch ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="펀치 쿨타임 (초)"))
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="대시 지속 시간 (초). 판정이 켜져 있는 시간. bDashDurationFromMontage면 무시"))
	float DashDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ToolTip="true면 대시 지속 시간을 PunchDashMontage 실제 재생 길이(Rate Scale 반영)로 맞춘다. 몽타주가 없으면 DashDuration 사용"))
	bool bDashDurationFromMontage = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch", meta=(ClampMin="0", ClampMax="89", ToolTip="대시 중 몸을 펀치 방향(위·아래)으로 기울이는 최대 각도(도). ABP가 DashPitch로 골반을 회전. 0이면 기울이지 않음"))
	float DashMaxVisualPitch = 60.f;

	// --- Punch 사거리: 대시 = 중력·감속 없는 일정 속도 직선 이동. 속도 = 거리 ÷ 대시 시간 ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Range", meta=(ClampMin="0", ToolTip="최소 충전(MinChargeTime) 시 대시 거리(cm)"))
	float PunchMinDistance = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Range", meta=(ClampMin="0", ToolTip="최대 충전(MaxChargeTime) 시 대시 거리(cm)"))
	float PunchMaxDistance = 2400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Range", meta=(ClampMin="0", ClampMax="1", ToolTip="대시가 끝까지 가서 끝났을 때 이어가는 속도 비율 (1=그대로 유지, 0=그 자리 정지). 공중이면 착지까지 유지. 적중·벽·착지 공격으로 끝나면 적용 안 함"))
	float PunchEndMomentumRatio = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Range", meta=(ClampMin="1", ToolTip="판정 캡슐 반지름(cm). 몸 앞에 내민 캡슐이 대시 중에만 켜짐. 기본 = 미카 캡슐(34)의 1.5배"))
	float PunchHitRadius = 51.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Range", meta=(ClampMin="1", ToolTip="판정 캡슐 반높이(cm). 기본 = 미카 캡슐(88)의 1.5배"))
	float PunchHitHalfHeight = 132.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Range", meta=(ToolTip="판정 캡슐을 몸 중심에서 앞으로 내미는 거리(cm)"))
	float PunchHitForwardOffset = 51.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Range", meta=(ToolTip="충전 중 바닥에 대시 경로를 보여주는 데칼 머티리얼 (Material Domain = Deferred Decal). 비우면 표시 안 함"))
	UMaterialInterface* PunchRangeDecalMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Range", meta=(ToolTip="충전 중 경로 끝(도착 예상 지점)에 폭발 범위(PunchExplosionMin~MaxRadius, 충전 비례)를 원으로 보여주는 데칼 머티리얼. 풀 충전 전 외곽선, 후 채움. 비우면 표시 안 함"))
	UMaterialInterface* PunchExplosionDecalMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Range", meta=(ToolTip="디버그: 판정 캡슐을 그린다 (충전 중 노랑, 대시 중 빨강). 착지 공격 반경은 1초간 구로 표시. 개발 빌드에서만"))
	bool bDebugDrawPunchHit = false;

	// --- Punch 적중: 충전 비율(0~1)로 Min~Max 보간 ---

	// 정면 적중: 대시 직선에서 적 중심까지 옆 거리 ≤ PunchFrontHalfWidth → 100% 피해, 미카 멈춤+반동, 적은 대시 방향으로 날아감, 메인 히트 FX
	// 측면 적중: 그 밖 → 측면 비율 피해, 미카 계속 진행, 적은 미카→적 방향으로 날아감, 서브 히트 FX

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ToolTip="최소 충전 시 피해량 (정면 100% 기준)"))
	float PunchMinDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ToolTip="최대 충전 시 피해량 (정면 100% 기준)"))
	float PunchMaxDamage = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ToolTip="정면 판정 반폭(cm). 대시 직선에서 적 중심까지의 옆 거리가 이 값 이하면 정면, 넘으면 측면"))
	float PunchFrontHalfWidth = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ToolTip="넉백 속도(cm/s) = 이번 대시 거리(cm) × 이 값. 1이면 24m 대시 → 2400cm/s"))
	float PunchKnockbackPerDistance = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ToolTip="넉백 위쪽 성분 비율 (0=수평, 0.3=살짝 띄움)"))
	float PunchKnockbackUpRatio = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ClampMax="1", ToolTip="측면 적중 피해 비율 (정면 대비)"))
	float PunchSideDamageRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ClampMax="1", ToolTip="측면 적중 넉백 비율 (정면 대비)"))
	float PunchSideKnockbackRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ToolTip="정면 적중·벽 반동의 세기. 반동 초기 속도 = 이번 대시 거리 × 이 값 ÷ PunchReboundTime. 이후 제동·중력으로 자연 감속 (실제 거리는 지상/공중에 따라 다름)"))
	float PunchReboundDistanceRatio = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0.01", ToolTip="반동 초기 속도 계산용 시간(초). 짧을수록 초기 속도가 커서 세게 튕김 (이 시간에 멈추는 게 아님)"))
	float PunchReboundTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ToolTip="풀 충전 대시: 정면 적중·벽에서 멈춘 뒤 반동 시작까지 제자리에 머무는 시간(게임 초). 히트스톱 중엔 그만큼 길게 느껴짐"))
	float PunchReboundDelayFull = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ToolTip="일반 충전(풀 충전 미만) 대시: 멈춘 뒤 반동 시작까지 제자리에 머무는 시간(게임 초)"))
	float PunchReboundDelayNormal = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0.01", ClampMax="1", ToolTip="메인 히트(정면·벽·착지 공격) 순간 게임 전체 시간 배속 (0.1 = 10% 속도). 히트스톱"))
	float PunchHitSlowTimeScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ToolTip="히트스톱 길이 (실제 시간, 초). 0이면 끔"))
	float PunchHitSlowDuration = 0.08f;

	// 폭발: 정면 적중·벽·착지 공격마다 항상 터짐. 반경·피해는 충전 비율로 Min~Max, 넉백은 펀치 넉백 × 비율

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Explosion", meta=(ClampMin="0", ToolTip="최소 충전 시 폭발 반경(cm). 충전할수록 Max까지 커짐 (범위 데칼 원도 같이)"))
	float PunchExplosionMinRadius = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Explosion", meta=(ClampMin="0", ToolTip="풀 충전 시 폭발 반경(cm). 0이면 폭발 피해·넉백 없음"))
	float PunchExplosionMaxRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Explosion", meta=(ClampMin="0", ToolTip="최소 충전 시 폭발 피해 (펀치 피해와 별도). 반경 안 전원(미카 제외)"))
	float PunchExplosionMinDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Explosion", meta=(ClampMin="0", ToolTip="풀 충전 시 폭발 피해 (펀치 피해와 별도)"))
	float PunchExplosionMaxDamage = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Explosion", meta=(ClampMin="0", ToolTip="폭발 넉백 = 이번 대시의 펀치 넉백 속도 × 이 값 (0.25 = 25%). 폭발 중심에서 바깥쪽으로. 정면으로 맞은 대상은 펀치 넉백이 덮어씀"))
	float PunchExplosionKnockbackRatio = 0.25f;

	// 착지 공격: 조준이 PunchSlamMinDownPitch 이상 아래이고 사거리 안에서 바닥에 닿으면(범위 원이 바닥에 뜨면) 그 지점까지 가서 발동

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ClampMax="89", ToolTip="착지 공격 최소 아래 각도(도). 이보다 얕게 아래를 보면 지상에선 수평 대시, 바닥 원도 안 뜸"))
	float PunchSlamMinDownPitch = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Hit", meta=(ClampMin="0", ToolTip="착지 범위 공격 반경(cm). 반경 안 전원에 충전 비례 피해·넉백"))
	float PunchSlamRadius = 300.f;

	// --- Punch FX: 단계 순서(충전 → 대시 → 히트)대로, 각 단계는 VFX → VFX 옵션 → SFX ---
	// 손 FX는 오른손 WeaponSocketRight에 부착, 해당 단계가 끝나면 정지

	// 1) 충전
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|1 Charge", meta=(ToolTip="충전 중 손에 붙는 나이아가라. 충전할수록 커짐. 충전이 끝나면 비활성화(남은 파티클은 자연 소멸)"))
	UNiagaraSystem* ChargeHandVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|1 Charge", meta=(ClampMin="0", ToolTip="충전 시작(MinChargeTime 이하) 시 충전 VFX 크기 배율"))
	float ChargeHandVFXMinScale = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|1 Charge", meta=(ClampMin="0", ToolTip="최대 충전(MaxChargeTime) 시 충전 VFX 크기 배율"))
	float ChargeHandVFXMaxScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|1 Charge", meta=(ClampMin="0", ToolTip="충전 VFX 원본 재생 길이(초, 나이아가라 에디터 타임라인에서 확인). 입력하면 ForcedMaxChargeTime 동안 재생되도록 느리게 재생. 0이면 원래 속도"))
	float ChargeHandVFXSourceLength = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|1 Charge", meta=(ClampMin="0", ToolTip="충전 VFX를 손에서 카메라 쪽으로 당기는 거리(cm). 몸에 가려지지 않고 캐릭터 앞에 보이게 함. 클수록 확실히 앞이지만 옆에서 보면 손에서 떠 보임. 0이면 손 위치 그대로"))
	float ChargeHandVFXCameraOffset = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|1 Charge", meta=(ToolTip="충전 중 손에서 나는 사운드. 충전이 끝나면 정지. 루프형 권장"))
	USoundBase* ChargeHandSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|1 Charge", meta=(ToolTip="풀 충전(MaxChargeTime) 도달 순간 1회 재생하는 사운드 (2D, UI 알림용)"))
	USoundBase* PunchFullChargeSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|1 Charge", meta=(ToolTip="풀 충전 도달 순간 오른손에 붙는 나이아가라. 풀 충전 펀치의 대시가 끝날 때까지 유지 (충전 VFX와 별개로 겹쳐 재생)"))
	UNiagaraSystem* PunchFullChargeHandVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|1 Charge", meta=(ClampMin="0", ToolTip="풀 충전 손 VFX 크기 배율"))
	float PunchFullChargeHandVFXScale = 1.f;

	// 2) 대시
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|2 Dash", meta=(ToolTip="대시 중 손에 붙는 나이아가라. 대시가 끝나면 비활성화(남은 파티클은 자연 소멸)"))
	UNiagaraSystem* DashHandVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|2 Dash", meta=(ToolTip="대시 중 손에서 나는 사운드. 대시가 끝나면 정지"))
	USoundBase* DashHandSFX = nullptr;

	// 3) 메인 히트 — 정면 적중·벽·착지 공격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|3 Main Hit", meta=(ToolTip="풀 충전 대시의 정면 적중·벽·착지 공격 위치에 재생하는 나이아가라. 미카 쪽을 향함. 대시당 1번 (풀 충전 미만이면 서브 히트 FX)"))
	UNiagaraSystem* PunchHitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|3 Main Hit", meta=(ToolTip="정면 적중·벽·착지 공격마다 터지는 폭발 나이아가라 (히트 VFX와 별개, 풀 충전 아니어도 터짐). 크기는 폭발 반경(충전 비례) / 기준 반경"))
	UNiagaraSystem* PunchExplosionVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|3 Main Hit", meta=(ClampMin="1", ToolTip="폭발 VFX가 크기 1일 때의 반경(cm). 폭발 반경과의 비율로 크기 조절 (수류탄 폭발과 같은 방식)"))
	float PunchExplosionVFXReferenceRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|3 Main Hit", meta=(ClampMin="0", ToolTip="최소 충전 시 메인 히트 VFX 크기 배율"))
	float PunchHitVFXMinScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|3 Main Hit", meta=(ClampMin="0", ToolTip="최대 충전 시 메인 히트 VFX 크기 배율"))
	float PunchHitVFXMaxScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|3 Main Hit", meta=(ToolTip="메인 히트 사운드. 대시당 1번"))
	USoundBase* PunchHitSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|3 Main Hit", meta=(ClampMin="0", ToolTip="최소 충전 시 메인 히트 사운드 볼륨 배율"))
	float PunchHitSFXMinVolume = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|3 Main Hit", meta=(ClampMin="0", ToolTip="최대 충전 시 메인 히트 사운드 볼륨 배율"))
	float PunchHitSFXMaxVolume = 1.f;

	// 4) 서브 히트 — 측면 적중
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|4 Side Hit", meta=(ToolTip="측면 적중 위치에 재생하는 나이아가라. 미카 쪽을 향함. 측면 대상마다"))
	UNiagaraSystem* PunchSideHitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|4 Side Hit", meta=(ClampMin="0", ToolTip="서브 히트 VFX 크기 배율"))
	float PunchSideHitVFXScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|FX|4 Side Hit", meta=(ToolTip="측면 적중 사운드. 측면 대상마다"))
	USoundBase* PunchSideHitSFX = nullptr;

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

	// 충전 진동: 감쇠(댐핑)·지연 없이 충전 비율만큼 커지는 흔들림. 카메라만 흔들어 조준 방향은 안 바뀜. 떼는 순간 즉시 정지

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera", meta=(ClampMin="0", ToolTip="풀 충전 시 카메라 위치 흔들림 폭(cm). 충전 비율만큼 0에서 커짐. 0이면 끔"))
	float ChargeShakeMaxLocation = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera", meta=(ClampMin="0", ToolTip="풀 충전 시 카메라 회전 흔들림 폭(도). 충전 비율만큼 0에서 커짐. 0이면 끔"))
	float ChargeShakeMaxRotation = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punch|Camera", meta=(ClampMin="0", ToolTip="흔들림 빠르기(Hz). 클수록 잘게 떨림"))
	float ChargeShakeFrequency = 20.f;

	// --- Animation ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta=(ToolTip="발사(반동) 시 캐릭터에 재생할 몽타주 (UpperBody 슬롯). 모든 무기 공통"))
	UAnimMontage* FireMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta=(ToolTip="앉은 상태 발사 몽타주. 비워두면 FireMontage를 그대로 쓴다"))
	UAnimMontage* FireMontageCrouch = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Grenade", meta=(ToolTip="수류탄 준비 몽타주(UpperBody 슬롯). 던지기 버튼을 누르고 있는 동안 유지되도록 Enable Auto Blend Out 해제 권장"))
	UAnimMontage* GrenadePrepareMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Grenade", meta=(ToolTip="수류탄 던지기 몽타주(UpperBody 슬롯). 준비 완료 후 버튼을 뗄 때 재생"))
	UAnimMontage* GrenadeThrowMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta=(ToolTip="재장전 몽타주(UpperBody 슬롯 → 상체만). 장전이 실제로 시작될 때만 재생. 길이는 무기 ReloadTime에 맞추는 걸 권장"))
	UAnimMontage* ReloadMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta=(ToolTip="true면 장전하는 동안 무기를 왼손 소켓으로 옮긴다 (무기의 LeftHandGrip 소켓이 손에 오도록 정렬)"))
	bool bAttachWeaponToLeftHandOnReload = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta=(ToolTip="장전 중 무기를 붙일 캐릭터 메시 소켓 이름 (미카 왼손 소켓)"))
	FName ReloadLeftHandSocket = TEXT("LeftHand");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta=(ToolTip="true면 무기 위치를 일정 프레임마다 구·선으로 그린다 (장전 중 총이 어디로 가는지 확인용)"))
	bool bDebugWeaponTrail = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Punch", meta=(ToolTip="펀치 충전 중 몽타주 (UpperBody 슬롯 — ABP에서 Layered blend per bone으로 상체만 적용). 충전이 길어질 수 있으니 반복 섹션으로 구성"))
	UAnimMontage* PunchChargeMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Punch", meta=(ToolTip="대시 펀치 몽타주. 제자리(비 Root) 애니 사용 — 이동은 코드가 담당"))
	UAnimMontage* PunchDashMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta=(ToolTip="펀치가 적·벽에 맞아 뒤로 튕겨 나올 때 재생할 몽타주 (구르기 등). 비우면 재생 안 함"))
	UAnimMontage* PunchReboundMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta=(ClampMin="0.1", ToolTip="펀치 반동 몽타주 재생 속도 배율. 1 = 원본, 0.7 = 30% 느리게"))
	float PunchReboundMontagePlayRate = 1.f;
};
