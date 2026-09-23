// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "MikaDataAsset.h"
#include "MikaCharacter.generated.h"

class UCapsuleComponent;
class UDecalComponent;
class UMaterialInterface;
class UHealthRegenComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;
class UAudioComponent;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPunchFullChargeSignature);

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AMikaCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	AMikaCharacter();

protected:
	virtual void BeginPlay() override;

	// 펀치·충전·수류탄 중에는 착지 모션을 붙잡지 않는다
	virtual bool IsLandPoseInterrupted() const override;

	virtual void PossessedBy(AController* NewController) override;

	// 카메라 위아래 제한각을 PlayerCameraManager에 적용. 빙의 시점이 BeginPlay보다 늦을 수 있어 양쪽에서 호출
	void ApplyCameraPitchLimits();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Camera", meta=(ToolTip="카메라 최저 피치(도). MikaData에서 설정"))
	float CameraPitchMin = -89.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Camera", meta=(ToolTip="카메라 최고 피치(도). MikaData에서 설정"))
	float CameraPitchMax = 89.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Camera", meta=(ToolTip="펀치 충전 중 카메라 최저 피치(도). MikaData에서 설정"))
	float ChargeCameraPitchMin = -89.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Camera", meta=(ToolTip="조준 중 허리 추가 회전(도). MikaData에서 설정"))
	float AimWaistYawOffset = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Camera", meta=(ToolTip="허리 틀기 보간 속도. MikaData에서 설정"))
	float AimWaistBlendSpeed = 10.f;

	// ABP용: 조준 중이면 AimWaistYawOffset 쪽으로, 아니면 0으로 보간되는 현재 허리 추가 회전(도)
	UPROPERTY(BlueprintReadOnly, Category = "Mika|Camera", meta=(ToolTip="조준 허리 틀기 현재값(도). ABP에서 Spine1 ModifyBone에 사용"))
	float AimWaistYaw = 0.f;
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

	// 몸 앞에 내민 캡슐 판정 (손에 붙이면 모션 궤적 전체가 판정이 돼 범위가 들쑥날쑥했다). 크기·앞 오프셋은 MikaData
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mika|Punch", meta=(ToolTip="펀치 판정 캡슐. 대시 중에만 켜짐"))
	UCapsuleComponent* PunchHitCapsule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="판정 캡슐 반지름(cm). MikaData에서 설정"))
	float PunchHitRadius = 51.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="판정 캡슐 반높이(cm). MikaData에서 설정"))
	float PunchHitHalfHeight = 132.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="판정 캡슐 앞 오프셋(cm). MikaData에서 설정"))
	float PunchHitForwardOffset = 51.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="디버그: 판정 캡슐·착지 공격 반경 표시. MikaData에서 설정"))
	bool bDebugDrawPunchHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="최소 충전 시 피해량. MikaData에서 설정"))
	float PunchMinDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="최대 충전 시 피해량. MikaData에서 설정"))
	float PunchMaxDamage = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="정면 판정 반폭(cm). MikaData에서 설정"))
	float PunchFrontHalfWidth = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="넉백 속도 = 대시 거리 × 이 값. MikaData에서 설정"))
	float PunchKnockbackPerDistance = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="넉백 위쪽 비율. MikaData에서 설정"))
	float PunchKnockbackUpRatio = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="측면 피해 비율. MikaData에서 설정"))
	float PunchSideDamageRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="측면 넉백 비율. MikaData에서 설정"))
	float PunchSideKnockbackRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="반동 거리 = 대시 거리 × 이 값. MikaData에서 설정"))
	float PunchReboundDistanceRatio = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="반동 이동 시간(초). MikaData에서 설정"))
	float PunchReboundTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="풀 충전 반동 딜레이(게임 초). MikaData에서 설정"))
	float PunchReboundDelayFull = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="일반 충전 반동 딜레이(게임 초). MikaData에서 설정"))
	float PunchReboundDelayNormal = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="대시 자연 종료 시 이어가는 속도 비율. MikaData에서 설정"))
	float PunchEndMomentumRatio = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="히트스톱 시간 배속. MikaData에서 설정"))
	float PunchHitSlowTimeScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="히트스톱 길이(실제 초, 0=끔). MikaData에서 설정"))
	float PunchHitSlowDuration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="충전 시작 시 폭발 반경(cm). MikaData에서 설정"))
	float PunchExplosionMinRadius = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="풀 충전 시 폭발 반경(cm). MikaData에서 설정"))
	float PunchExplosionMaxRadius = 300.f;

	float GetPunchExplosionRadius(float ChargeRatio) const { return FMath::Lerp(PunchExplosionMinRadius, PunchExplosionMaxRadius, ChargeRatio); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="최소 충전 폭발 피해. MikaData에서 설정"))
	float PunchExplosionMinDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="풀 충전 폭발 피해. MikaData에서 설정"))
	float PunchExplosionMaxDamage = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="폭발 넉백 = 펀치 넉백 × 이 값. MikaData에서 설정"))
	float PunchExplosionKnockbackRatio = 0.25f;

	// 메인 히트 순간 전역 시간을 늦추고, 실제 시간 PunchHitSlowDuration 뒤 복구
	void StartHitSlow();
	void EndHitSlow();
	FTimerHandle HitSlowTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="착지 범위 공격 반경(cm). MikaData에서 설정"))
	float PunchSlamRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="펀치 쿨타임 (초)"))
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="최소 충전 시 대시 거리(cm). MikaData에서 설정"))
	float PunchMinDistance = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="최대 충전 시 대시 거리(cm). MikaData에서 설정"))
	float PunchMaxDistance = 2400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="대시 지속 시간 (초). 판정이 켜져 있는 시간"))
	float DashDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="true면 대시 지속 시간 = 펀치 몽타주 재생 길이. MikaData에서 설정"))
	bool bDashDurationFromMontage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="대시 중 몸 기울기 최대 각도(도). MikaData에서 설정"))
	float DashMaxVisualPitch = 60.f;

	// --- Punch FX ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="충전 중 손 VFX. MikaData에서 설정"))
	UNiagaraSystem* ChargeHandVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="충전 시작 시 충전 VFX 크기. MikaData에서 설정"))
	float ChargeHandVFXMinScale = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="최대 충전 시 충전 VFX 크기. MikaData에서 설정"))
	float ChargeHandVFXMaxScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="충전 VFX 원본 길이(초). 0이면 원래 속도. MikaData에서 설정"))
	float ChargeHandVFXSourceLength = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="충전 VFX를 카메라 쪽으로 당기는 거리(cm). MikaData에서 설정"))
	float ChargeHandVFXCameraOffset = 40.f;

	// 충전 VFX를 손 위치에서 카메라 쪽으로 ChargeHandVFXCameraOffset만큼 옮김 (매 프레임)
	void UpdateChargeVFXCameraOffset();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="대시 중 손 VFX. MikaData에서 설정"))
	UNiagaraSystem* DashHandVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="충전 중 손 SFX. MikaData에서 설정"))
	USoundBase* ChargeHandSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="풀 충전 도달 SFX. MikaData에서 설정"))
	USoundBase* PunchFullChargeSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="풀 충전 손 VFX. MikaData에서 설정"))
	UNiagaraSystem* PunchFullChargeHandVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="풀 충전 손 VFX 크기. MikaData에서 설정"))
	float PunchFullChargeHandVFXScale = 1.f;

	// 풀 충전 손 VFX — 충전·대시 손 VFX(ActiveHandVFX)와 겹쳐 재생, 풀 충전 대시가 끝날 때 정지
	UPROPERTY(Transient)
	UNiagaraComponent* ActiveFullChargeVFX = nullptr;
	void StopFullChargeVFX();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="메인 히트 폭발 VFX. MikaData에서 설정"))
	UNiagaraSystem* PunchExplosionVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="폭발 VFX 기준 반경(cm). MikaData에서 설정"))
	float PunchExplosionVFXReferenceRadius = 300.f;

	// 메인 히트 폭발: VFX(반경 비례 크기) + 반경 피해
	void PunchExplode(const FVector& Location);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="대시 중 손 SFX. MikaData에서 설정"))
	USoundBase* DashHandSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="대시당 첫 히트 VFX. MikaData에서 설정"))
	UNiagaraSystem* PunchHitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="최소 충전 시 메인 히트 VFX 크기. MikaData에서 설정"))
	float PunchHitVFXMinScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="최대 충전 시 메인 히트 VFX 크기. MikaData에서 설정"))
	float PunchHitVFXMaxScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="최소 충전 시 메인 히트 SFX 볼륨. MikaData에서 설정"))
	float PunchHitSFXMinVolume = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="최대 충전 시 메인 히트 SFX 볼륨. MikaData에서 설정"))
	float PunchHitSFXMaxVolume = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="메인 히트 SFX. MikaData에서 설정"))
	USoundBase* PunchHitSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="측면 히트 VFX. MikaData에서 설정"))
	UNiagaraSystem* PunchSideHitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="측면 히트 VFX 크기. MikaData에서 설정"))
	float PunchSideHitVFXScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|FX", meta=(ToolTip="측면 히트 SFX. MikaData에서 설정"))
	USoundBase* PunchSideHitSFX = nullptr;

	// 재생 중인 손 FX
	UPROPERTY(Transient)
	UNiagaraComponent* ActiveHandVFX = nullptr;

	UPROPERTY(Transient)
	UAudioComponent* ActiveHandSFX = nullptr;

	// 오른손 VFX 시작(기존 손 VFX는 먼저 정지)·충전 크기·정지. 크기는 충전 비율(0~1)로 Min~Max 보간
	void StartHandVFX(UNiagaraSystem* VFX);
	void SetChargeVFXScale(float ChargeRatio);
	void StopHandVFX();

	// 오른손 SFX (기존 손 SFX는 먼저 정지)
	void PlayHandSFX(USoundBase* SFX);
	void StopHandSFX();

	// 충전 시간 → 0~1 (대시 속도와 같은 기준)
	float GetChargeRatio() const;

	// 메인 히트 FX(정면·벽·착지 공격)는 대시당 1번. 서브(측면)는 대상마다
	bool bDashHitFXPlayed = false;
	void PlayPunchHitFX(const FVector& Location);   // 메인. VFX는 미카 쪽을 향함
	void PlayPunchSideHitFX(const FVector& Location);
	void SpawnHitFX(UNiagaraSystem* VFX, float Scale, USoundBase* SFX, float Volume, const FVector& Location);

	// 대시 중 캡슐이 벽에 막힐 때 히트 FX (벽은 오버랩 이벤트가 없어 히트박스로 못 잡음)
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera", meta=(ToolTip="풀 충전 카메라 위치 흔들림(cm). MikaData에서 설정"))
	float ChargeShakeMaxLocation = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera", meta=(ToolTip="풀 충전 카메라 회전 흔들림(도). MikaData에서 설정"))
	float ChargeShakeMaxRotation = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Camera", meta=(ToolTip="흔들림 빠르기(Hz). MikaData에서 설정"))
	float ChargeShakeFrequency = 20.f;

	// 충전 진동: 카메라 컴포넌트의 상대 위치·회전에 노이즈를 더한다 (감쇠 없음). 원래 값은 BeginPlay에서 저장
	FVector  CameraBaseRelLocation = FVector::ZeroVector;
	FRotator CameraBaseRelRotation = FRotator::ZeroRotator;
	void UpdateChargeShake(float ChargeRatio);   // 충전 중 매 프레임
	void StopChargeShake();                      // 원래 위치로 즉시 복귀

	// --- Punch Range Decal ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mika|Punch|Range", meta=(ToolTip="충전 중 바닥에 대시 경로를 보여주는 데칼"))
	UDecalComponent* PunchRangeDecal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Range", meta=(ToolTip="범위 데칼 머티리얼. MikaData에서 설정"))
	UMaterialInterface* PunchRangeDecalMaterial = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mika|Punch|Range", meta=(ToolTip="충전 중 경로 끝에 폭발 범위를 보여주는 원형 데칼"))
	UDecalComponent* PunchExplosionDecal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Range", meta=(ToolTip="폭발 범위 데칼 머티리얼. MikaData에서 설정"))
	UMaterialInterface* PunchExplosionDecalMaterial = nullptr;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* PunchExplosionDecalMID = nullptr;
	float PunchExplosionDecalFillOpacity = 0.25f;

	// 충전 중 매 프레임: 지금 떼면 대시할 경로를 바닥에 사각형 데칼로 표시
	void UpdatePunchRangeDecal(float ChargeRatio);

	// 풀 충전 전 = 외곽선만(FillOpacity 0), 풀 충전 후 = 머티리얼 기본 채움
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* PunchRangeDecalMID = nullptr;
	float PunchRangeDecalFillOpacity = 0.25f;   // 머티리얼의 FillOpacity 기본값 (BeginPlay에서 읽음)
	void SetPunchRangeDecalFilled(bool bFilled);

public:
	// UI용: 충전 중 풀 충전(MaxChargeTime)에 도달한 순간 1회
	UPROPERTY(BlueprintAssignable, Category = "Mika|Punch")
	FOnPunchFullChargeSignature OnPunchFullCharge;

	// UI 게이지용 충전 비율 (0~1). 충전 중이 아니면 0
	UFUNCTION(BlueprintPure, Category = "Mika|Punch")
	float GetPunchChargeRatio() const { return bIsChargingPunch ? GetChargeRatio() : 0.f; }

protected:
	bool bFullChargeNotified = false;   // 이번 충전에서 풀 충전 알림을 보냈는지
	bool bDashFullCharge     = false;   // 이번 대시가 풀 충전인지 → 메인 히트 연출(메인 FX·히트스톱·폭발) 여부

	// --- Punch Animation ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Animation", meta=(ToolTip="펀치 충전 몽타주. MikaData에서 설정"))
	UAnimMontage* PunchChargeMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch|Animation", meta=(ToolTip="대시 펀치 몽타주. MikaData에서 설정"))
	UAnimMontage* PunchDashMontage = nullptr;

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State", meta=(ToolTip="펀치 사용 가능 여부. 쿨타임 중이면 false"))
	bool bCanPunch        = true;

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State", meta=(ToolTip="현재 펀치 충전 중인지"))
	bool bIsChargingPunch = false;

	UPROPERTY(BlueprintReadOnly, Category = "Mika|State", meta=(ToolTip="현재 대시 중인지"))
	bool bIsDashing       = false;

	// 대시가 적중·벽 반동으로 일찍 끝나도 펀치 몽타주가 끝날 때까지 하반신까지 펀치 모션을 유지 (ABP 전신 분기용)
	UPROPERTY(BlueprintReadOnly, Category = "Mika|State", meta=(ToolTip="대시 시작 ~ 펀치 몽타주 끝까지 true. ABP 전신 분기에 사용"))
	bool bIsPunchFullBody = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Punch", meta=(ToolTip="펀치 반동(뒤로 튕김) 몽타주. MikaData에서 설정"))
	UAnimMontage* PunchReboundMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Punch", meta=(ClampMin="0.1", ToolTip="펀치 반동 몽타주 재생 속도 배율. MikaData에서 설정"))
	float PunchReboundMontagePlayRate = 1.f;

	FTimerHandle PunchFullBodyTimerHandle;
	void EndPunchFullBody();

	// 대시 방향의 상하 각도(도, 위=+). 캡슐은 수직을 유지해야 하므로 액터가 아니라 ABP에서 골반만 이만큼 기울인다
	UPROPERTY(BlueprintReadOnly, Category = "Mika|State", meta=(ToolTip="펀치 방향 Pitch(도, 위=+). 대시 시작 ~ 펀치 몽타주 끝까지 유지, 그 외 0. ABP 골반 회전용"))
	float DashPitch = 0.f;

	float ChargeStartTime            = 0.f;
	float NormalSpringArmLength      = 300.f;
	float DefaultBrakingDeceleration     = 2048.f;
	float DefaultBrakingDecelerationFly  = 0.f;
	float DefaultBrakingDecelerationFall = 0.f;
	float DefaultBrakingFrictionFactor   = 2.f;

	// 현재 대시: 일정 속도 직선 이동(비행 모드) — 거리 = 속도 × 시간이 정확히 지켜지게
	FVector DashVelocity     = FVector::ZeroVector;
	float   DashChargeRatio  = 0.f;   // 대미지 비례용
	float   DashDistance     = 0.f;   // 넉백·반동 비례용 (이번 대시의 계획 거리)

	// 정면 적중 반동: 딜레이 동안 제자리(비행 모드) → 반대 방향 초기 속도만 주고 이후는 물리(제동·중력)에 맡겨 자연 감속
	// 충돌 콜백은 이동 처리 도중에 불리므로, 거기서 이동 모드를 바꾸지 않고 다음 Tick에서 반동 시작
	bool    bPendingRebound  = false;
	bool    bIsRebounding    = false;   // 반동 딜레이 중 (Tick이 제자리 유지)
	FVector PendingReboundVelocity = FVector::ZeroVector;
	FTimerHandle ReboundDelayTimerHandle;
	void BeginReboundMove();   // 딜레이가 끝나면 반동 초기 속도를 주고 낙하 모드로

	TSet<AActor*> HitActorsDuringDash;

	FTimerHandle DashEndTimerHandle;
	FTimerHandle PunchCooldownTimerHandle;
	FTimerHandle AutoReleaseTimerHandle;
	FTimerHandle WeaponRestoreTimerHandle;   // 펀치·착지 몽타주가 끝난 뒤 총 복구

	// --- Overrides ---

	virtual void StartFire() override;
	virtual void StopFire() override;

	// --- Internal ---

	// 지금 조준 방향 (카메라 시선). 대시·범위 데칼 공용
	FVector GetPunchAimDirection() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mika|Punch", meta=(ToolTip="착지 공격 최소 아래 각도(도). MikaData에서 설정"))
	float PunchSlamMinDownPitch = 30.f;

	// 경로 끝의 종류 — Floor: 착지 공격 지점, Blocked: 벽·장애물(반동+폭발), Air: 허공(폭발 없음)
	enum class EPunchPathEnd : uint8 { Air, Floor, Blocked };

	// 지금 떼면 갈 경로 (범위 데칼과 대시가 같은 결과를 쓴다). OutDir = 실제 이동 방향:
	// 지상에서 PunchSlamMinDownPitch보다 얕게 아래를 보면 수평으로 눌러 바닥을 비비며 미끄러지지 않게 한다
	// OutNormal = 경로 끝 표면 방향 (범위 원 데칼을 그 면에 투사). 적에 막히면 위(바닥에 투사)
	EPunchPathEnd TracePunchPath(float ChargeRatio, FVector& OutEnd, FVector& OutDir, FVector& OutNormal) const;

	// 바닥 목표 대시: 시작 때 경로가 바닥에 닿으면 그 지점까지 가서 착지 공격 (원이 보이는 곳 = 공격 지점)
	bool    bDashFloorTarget   = false;
	FVector DashFloorTarget    = FVector::ZeroVector;
	FVector DashStartLocation  = FVector::ZeroVector;
	float   DashFloorTravel    = 0.f;     // 목표까지 이동할 거리 (지상 출발이면 수평 거리)
	bool    bDashFloorFlat     = false;   // 지상 출발 → 바닥을 따라 수평 이동
	bool    bPendingSlam       = false;   // 충돌 콜백(이동 처리 중)에서 받은 착지 공격을 다음 Tick에 실행
	FVector PendingSlamPoint   = FVector::ZeroVector;

	void StartDash(float ChargeRatio);
	void EndDash();                          // 적중·벽·착지 공격: 그 자리 정지
	void EndDashKeepMomentum();              // 끝까지 간 자연 종료: 속도 × PunchEndMomentumRatio 유지
	void FinishDash(bool bKeepMomentum);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void StartPunchCooldown();
	void ResetPunchCooldown();

	// 한 대상에 충전 비례 대미지 × DamageScale + 넉백(방향 × 대시 거리 비례 속도 × KnockbackScale). 파괴물은 즉시 파괴
	void ApplyPunchHit(AActor* Target, UPrimitiveComponent* TargetComp, const FVector& KnockbackDir, float DamageScale, float KnockbackScale);

	// 대시 직선에서 대상 중심까지의 옆 거리가 PunchFrontHalfWidth 이하인지
	bool IsFrontHit(const AActor* Target) const;

	// 정면 적중: 대시를 끊고 대시 거리 × PunchReboundDistanceRatio만큼 반대 방향으로 튕김
	void ReboundFromHit();

	// 아래로 대시하다 바닥에 닿으면 그 지점 반경 내 전원에 ApplyPunchHit
	void PunchSlam(const FVector& ImpactPoint);

	// 몽타주 길이만큼 기다렸다 총 복구 — 대시(0.25초)보다 펀치 애니가 길어 대시 종료 시점에 복구하면 총이 거의 안 숨는다
	void RestoreWeaponAfter(float Delay);
	void RestoreWeapon() { SetCurrentWeaponHidden(false); }
	float PunchMontageEndTime = 0.f;


	UFUNCTION()
	void OnPunchHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
