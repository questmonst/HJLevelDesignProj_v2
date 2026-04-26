// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "CharacterTypes.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UArrowComponent;
class AWeaponBase;
class UHUDDataAsset;
class AGrenadeBase;
class UInputMappingContext;
class UInputAction;
class USplineComponent;
class USplineMeshComponent;
class UTraversalComponent;
class UMotionWarpingComponent;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Traversal")
	UTraversalComponent* TraversalComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Traversal")
	UMotionWarpingComponent* MotionWarpingComponent;

	// --- HUD Data ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|HUD")
	UHUDDataAsset* HUDData = nullptr;

	// --- Input ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Input")
	UInputMappingContext* IMC_Default;

	// --- Input Handlers ---

	const UInputAction* FindActionInIMC(const FString& NameContains) const;
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void StartCrouch();
	void StopCrouch();

	/** 트래버설 감지 후 가능하면 트래버설, 불가능하면 일반 점프 */
	virtual void Jump() override;

	// --- Action Permission ---

	/** 현재 허용된 행동 비트마스크. ABP/BP에서 읽기 가능. */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Action")
	int32 AllowedActions;

	/** 해당 행동이 현재 허용되어 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Character|Action")
	bool CanDo(ECharacterAction Action) const;

	/** 특정 행동을 허용 (AnimNotify, 상태 종료 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Character|Action")
	void Allow(ECharacterAction Action);

	/** 특정 행동을 금지 (상태 진입 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Character|Action")
	void Block(ECharacterAction Action);

	// --- Movement ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement")
	float SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement")
	float AimWalkSpeed;

	// --- Crosshair ---

	/** WBP에서 크로스헤어 크기/퍼짐에 사용 */
	UPROPERTY(BlueprintReadOnly, Category = "Character|HUD")
	float CurrentCrosshairSpread = 0.f;

	// --- Aim ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Aim")
	UArrowComponent* AimArrow;

	/** AnimBP에서 읽어 spine Transform Modify Bone에 적용할 피치 값 */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Aim")
	float AimSpinePitch = 0.f;

	/** AimSpinePitch 클램프 범위 (±도) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Aim")
	float AimSpinePitchClamp = 60.f;

	/** AimSpinePitch 보간 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Aim")
	float AimSpineInterpSpeed = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float NormalFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float AimFOV;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Combat")
	bool bIsAiming;

	// --- Turn In Place ---

	/** 캐릭터 액터 Yaw와 카메라 Yaw의 차이 (-180~180). ABP Rotate Root Bone에 사용. */
	UPROPERTY(BlueprintReadOnly, Category = "Character|TurnInPlace")
	float AimYaw = 0.f;

	/** 오른쪽 Turn 발동 각도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|TurnInPlace")
	float TurnRightThreshold = 90.f;

	/** 왼쪽 Turn 발동 각도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|TurnInPlace")
	float TurnLeftThreshold = 90.f;

	/** Turn 애니 중 액터 회전 속도 (도/초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|TurnInPlace")
	float TurnRotationSpeed = 200.f;

	/** 임계값 미만일 때 카메라를 따라가는 속도 (도/초). 0이면 비활성화. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|TurnInPlace")
	float SoftTurnSpeed = 60.f;

	/** ABP에서 AimTurnR 애니메이션 트리거 */
	UPROPERTY(BlueprintReadOnly, Category = "Character|TurnInPlace")
	bool bIsTurningRight = false;

	/** ABP에서 AimTurnL 애니메이션 트리거 */
	UPROPERTY(BlueprintReadOnly, Category = "Character|TurnInPlace")
	bool bIsTurningLeft = false;

	void UpdateAimTurn(float DeltaTime);

	// --- Cover Peek ---

	/** 좌우 엄폐물 감지 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float CoverTraceDistance = 80.f;

	/** 엄폐 시 카메라 오프셋 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float CoverPeekOffset = 80.f;

	/** 카메라 오프셋 보간 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float CoverPeekInterpSpeed = 8.f;

	/** 기본 카메라 좌우 오프셋 (양수 = 오른쪽) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float DefaultSocketOffsetY = 60.f;

	void UpdateCoverPeek(float DeltaTime);
	void UpdateAimSpinePitch(float DeltaTime);
	void UpdateCrosshairSpread(float DeltaTime);

	float SpreadAdditive = 0.f;

private:
	float NormalSocketOffsetY = 0.f;

protected:

	// --- Grenade ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade")
	TSubclassOf<AGrenadeBase> GrenadeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade")
	float GrenadeThrowSpeed = 1200.f;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Grenade")
	int32 GrenadeCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade")
	UStaticMesh* TrajectoryMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade")
	UMaterialInterface* TrajectoryMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade")
	float TrajectoryMeshScale = 0.1f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Grenade")
	USplineComponent* TrajectorySpline;

	bool bIsPreparingThrow = false;

	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshPool;

	static const int32 MaxTrajectorySegments = 20;

	void StartGrenadeThrow();
	void ReleaseGrenadeThrow();
	void UpdateTrajectory();
	void ClearTrajectory();

	// --- Weapon ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon")
	TArray<TSubclassOf<AWeaponBase>> DefaultWeaponClasses;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon")
	TArray<AWeaponBase*> WeaponInventory;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon")
	AWeaponBase* CurrentWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon")
	int32 CurrentWeaponIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon")
	int32 MaxWeaponSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon")
	FName WeaponAttachSocket;

	/** 수납 시 붙일 소켓 (등, 허리 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon")
	FName WeaponHolsterSocket;

	/** 무기 교체 딜레이 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon")
	float WeaponSwapDelay = 0.3f;

	bool  bIsSwapping        = false;
	int32 PendingWeaponIndex = -1;

	FTimerHandle SwapTimerHandle;

	void FinishEquipWeapon();

public:
	// --- Movement ---

	UFUNCTION(BlueprintCallable, Category = "Character|HUD")
	void AddCrosshairSpread(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void StopSprint();

	// --- Aim ---

	UFUNCTION(BlueprintCallable, Category = "Character|Camera")
	void StartAim();

	UFUNCTION(BlueprintCallable, Category = "Character|Camera")
	void StopAim();

	// --- Weapon ---

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	virtual void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	virtual void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	void EquipWeapon(int32 Index);

	void EquipWeaponSlot1();
	void EquipWeaponSlot2();
	void EquipWeaponSlot3();

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	bool PickupWeapon(AWeaponBase* Weapon);

	UFUNCTION(BlueprintPure, Category = "Character|Weapon")
	AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

	// --- Gravity ---

	/**
	 * 중력 방향을 변경합니다.
	 * 레벨 블루프린트에서 Sequencer 특정 타이밍에 호출.
	 * @param NewDirection  새 중력 방향 벡터 (정규화 불필요, 내부에서 처리)
	 *                      예) (0,0,-1) = 기본, (0,0,1) = 천장이 바닥, (1,0,0) = 오른쪽 벽이 바닥
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Gravity")
	void SetGravityDirection(FVector NewDirection);

	/** 중력을 기본값 (0, 0, -1) 으로 복원합니다. */
	UFUNCTION(BlueprintCallable, Category = "Character|Gravity")
	void ResetGravity();

	// ────────────────────────────────────────────────
	// Fall / Landing (AnimBP 연동)
	// ────────────────────────────────────────────────

	/** 현재 공중에서 낙하 중인지 (점프 상승 포함) — AnimBP에서 읽기 */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall")
	bool bIsFalling = false;

	/** 낙하 속도 (cm/s, 양수). 상승 중이면 0. AnimBP 블렌딩에 사용 */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall")
	float CurrentFallSpeed = 0.f;

	/** 이 속도(cm/s) 이상으로 착지하면 하드 랜딩 판정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Fall")
	float HardLandingSpeedThreshold = 600.f;

	/** 하드 랜딩 직후 true. ResetHardLanding() 호출 전까지 유지. */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall")
	bool bIsHardLanding = false;

	/**
	 * 착지 이벤트. BP에서 오버라이드하여 랜딩 몽타주 재생.
	 * @param bHardLanding  하드 랜딩이면 true (HardLandingSpeedThreshold 이상 속도)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character|Fall")
	void OnLanding(bool bHardLanding);
	virtual void OnLanding_Implementation(bool bHardLanding);

	/**
	 * 하드 랜딩 애니메이션이 끝난 뒤 AnimNotify에서 호출.
	 * bIsHardLanding 플래그를 리셋한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Fall")
	void ResetHardLanding();

protected:
	/** ACharacter 기본 착지 콜백 — 내부에서 OnLanding 발동 */
	virtual void Landed(const FHitResult& Hit) override;
};
