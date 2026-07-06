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
class UAnimMontage;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera", meta=(ToolTip="스프링 암 컴포넌트"))
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera", meta=(ToolTip="카메라 컴포넌트"))
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Traversal", meta=(ToolTip="트래버설 컴포넌트"))
	UTraversalComponent* TraversalComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Traversal", meta=(ToolTip="모션 워핑 컴포넌트"))
	UMotionWarpingComponent* MotionWarpingComponent;

	// --- HUD Data ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|HUD", meta=(ToolTip="HUD 설정 데이터 에셋"))
	UHUDDataAsset* HUDData = nullptr;

	// --- Input ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Input", meta=(ToolTip="기본 입력 매핑 컨텍스트"))
	UInputMappingContext* IMC_Default;

	// --- Input Handlers ---

	const UInputAction* FindActionInIMC(const FString& NameContains) const;
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	// 앉기는 홀드가 아닌 토글: 누를 때마다 앉기<->서기 전환
	void ToggleCrouch();

	virtual void Jump() override;

	// --- Action Permission ---

	UPROPERTY(BlueprintReadOnly, Category = "Character|Action", meta=(ToolTip="현재 허용된 행동 비트마스크. ABP/BP에서 읽기 가능"))
	int32 AllowedActions;

	UFUNCTION(BlueprintPure, Category = "Character|Action")
	bool CanDo(ECharacterAction Action) const;

	UFUNCTION(BlueprintCallable, Category = "Character|Action")
	void Allow(ECharacterAction Action);

	UFUNCTION(BlueprintCallable, Category = "Character|Action")
	void Block(ECharacterAction Action);

	// --- Movement ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement", meta=(ToolTip="걷기 속도 (cm/s)"))
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement", meta=(ToolTip="달리기 속도 (cm/s)"))
	float SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement", meta=(ToolTip="조준 중 걷기 속도 (cm/s)"))
	float AimWalkSpeed;

	// --- Crosshair ---

	UPROPERTY(BlueprintReadOnly, Category = "Character|HUD", meta=(ToolTip="WBP에서 크로스헤어 크기·퍼짐에 사용"))
	float CurrentCrosshairSpread = 0.f;

	// --- Aim ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Aim", meta=(ToolTip="조준 방향 화살표 컴포넌트. BP 뷰포트에서 머즐 소켓 위치에 배치"))
	UArrowComponent* AimArrow;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Aim", meta=(ToolTip="AnimBP에서 spine Transform Modify Bone에 적용할 피치 값"))
	float AimSpinePitch = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Aim", meta=(ToolTip="AimSpinePitch 클램프 범위 (±도). 상하 조준 제한각"))
	float AimSpinePitchClamp = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Aim", meta=(ToolTip="AimSpinePitch 보간 속도. 클수록 상하 조준이 빠르게 반응"))
	float AimSpineInterpSpeed = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="기본 카메라 시야각 (도)"))
	float NormalFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="조준 시 카메라 시야각 (도). 낮을수록 줌인"))
	float AimFOV;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Combat", meta=(ToolTip="현재 조준(ADS) 상태 여부"))
	bool bIsAiming;

	// --- Turn In Place ---

	UPROPERTY(BlueprintReadOnly, Category = "Character|TurnInPlace", meta=(ToolTip="캐릭터 액터 Yaw와 카메라 Yaw의 차이 (-180~180). ABP Rotate Root Bone에 사용"))
	float AimYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|TurnInPlace", meta=(ToolTip="오른쪽 Turn 발동 각도 (도). 카메라가 이 각도 이상 오른쪽으로 벌어지면 회전 시작"))
	float TurnRightThreshold = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|TurnInPlace", meta=(ToolTip="왼쪽 Turn 발동 각도 (도)"))
	float TurnLeftThreshold = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|TurnInPlace", meta=(ToolTip="Turn 애니 중 액터 회전 속도 (도/초)"))
	float TurnRotationSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|TurnInPlace", meta=(ToolTip="임계값 미만일 때 카메라를 따라가는 속도 (도/초). 0이면 비활성화"))
	float SoftTurnSpeed = 60.f;

	UPROPERTY(BlueprintReadOnly, Category = "Character|TurnInPlace", meta=(ToolTip="ABP에서 AimTurnR 애니메이션 트리거"))
	bool bIsTurningRight = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character|TurnInPlace", meta=(ToolTip="ABP에서 AimTurnL 애니메이션 트리거"))
	bool bIsTurningLeft = false;

	void UpdateAimTurn(float DeltaTime);

	// --- Cover Peek ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="좌우 엄폐물 감지 거리 (cm)"))
	float CoverTraceDistance = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="엄폐 시 카메라가 옆으로 이동하는 거리 (cm)"))
	float CoverPeekOffset = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="엄폐 카메라 이동 보간 속도. 클수록 빠르게 전환"))
	float CoverPeekInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="기본 카메라 소켓 좌우 오프셋 (양수=오른쪽, cm)"))
	float DefaultSocketOffsetY = 60.f;

	void UpdateCoverPeek(float DeltaTime);
	void UpdateAimSpinePitch(float DeltaTime);
	void UpdateCrosshairSpread(float DeltaTime);
	void UpdateRecoil(float DeltaTime);

	float SpreadAdditive   = 0.f;
	float RecoilPitchAccum = 0.f;

private:
	float NormalSocketOffsetY = 0.f;

protected:

	// --- Grenade ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade", meta=(ToolTip="스폰할 수류탄 클래스"))
	TSubclassOf<AGrenadeBase> GrenadeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade", meta=(ToolTip="수류탄 초기 투사 속도 (cm/s)"))
	float GrenadeThrowSpeed = 1200.f;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Grenade", meta=(ToolTip="현재 수류탄 보유 개수"))
	int32 GrenadeCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade", meta=(ToolTip="수류탄 궤적 시각화에 사용할 스태틱 메시"))
	UStaticMesh* TrajectoryMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade", meta=(ToolTip="수류탄 궤적 메시에 적용할 머티리얼"))
	UMaterialInterface* TrajectoryMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade", meta=(ToolTip="궤적 메시 스케일"))
	float TrajectoryMeshScale = 0.1f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Grenade", meta=(ToolTip="수류탄 궤적 스플라인 컴포넌트"))
	USplineComponent* TrajectorySpline;

	bool bIsPreparingThrow = false;

	UPROPERTY()
	AGrenadeBase* HeldGrenade = nullptr;	// 조준 중 오른손에 들고 있는 수류탄 (Release 시 발사)

	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshPool;

	static const int32 MaxTrajectorySegments = 20;

	void StartGrenadeThrow();
	void ReleaseGrenadeThrow();
	void UpdateTrajectory();			// 수류탄 투척 궤도 (시작점·속도 계산 후 RenderTrajectory 호출)
	void UpdateWeaponTrajectory();		// 유탄 발사기 궤도 (시작점·속도 계산 후 RenderTrajectory 호출)
	void RenderTrajectory(const FVector& Start, const FVector& Velocity);	// 공용 궤도 렌더러
	void ClearTrajectory();

	// --- Weapon ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon", meta=(ToolTip="초기 장착할 무기 클래스 목록"))
	TArray<TSubclassOf<AWeaponBase>> DefaultWeaponClasses;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon", meta=(ToolTip="범위 안에 있는 픽업 가능한 무기. HUD에서 프롬프트 표시에 사용"))
	AWeaponBase* PendingPickupWeapon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon", meta=(ToolTip="픽업 프롬프트 표시 여부. WBP에서 바인딩"))
	bool bShowPickupPrompt = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon", meta=(ToolTip="현재 무기 인벤토리"))
	TArray<AWeaponBase*> WeaponInventory;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon", meta=(ToolTip="현재 장착된 무기"))
	AWeaponBase* CurrentWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon", meta=(ToolTip="현재 장착 무기 인덱스"))
	int32 CurrentWeaponIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon", meta=(ToolTip="최대 무기 슬롯 수"))
	int32 MaxWeaponSlots;

	FName WeaponAttachSocket  = TEXT("WeaponSocketRight");
	// None이면 비장착(홀스터) 무기를 숨김 — 손에 든 무기만 표시.
	// 몸에 무기를 보이게 하려면 등·허리 등 별도 홀스터 소켓 이름을 지정.
	FName WeaponHolsterSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon", meta=(ToolTip="무기 교체 딜레이 (초). 이 시간 후 새 무기가 장착됨"))
	float WeaponSwapDelay = 0.3f;

	// 발사(반동) 몽타주 — 무기별이 아닌 캐릭터 공통. MikaData에서 주입 (UpperBody 슬롯)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Animation", meta=(ToolTip="발사(반동) 시 재생할 몽타주. 모든 무기 공통"))
	UAnimMontage* FireMontage = nullptr;

	bool  bIsSwapping        = false;
	bool  bIsFiring          = false;
	int32 PendingWeaponIndex = -1;
	int32 LastWeaponIndex    = -1;

	FTimerHandle SwapTimerHandle;

	void FinishEquipWeapon();

public:
	// --- Movement ---

	UFUNCTION(BlueprintCallable, Category = "Character|HUD")
	void AddCrosshairSpread(float Amount);

	void ApplyRecoilShot();

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
	void SwapToLastWeapon();

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	bool PickupWeapon(AWeaponBase* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	void DropCurrentWeapon();

	// 슬롯이 꽉 찼을 때: 현재 든 무기를 같은 슬롯에서 땅의 무기와 교체 (인덱스 유지)
	void SwapCurrentWeaponWith(AWeaponBase* NewWeapon);

	void OnWeaponPickupRangeEnter(AWeaponBase* Weapon);
	void OnWeaponPickupRangeExit(AWeaponBase* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	void TryPickupNearbyWeapon();

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Weapon")
	void OnPickupPromptChanged(bool bShow, AWeaponBase* Weapon);
	virtual void OnPickupPromptChanged_Implementation(bool bShow, AWeaponBase* Weapon);

	UFUNCTION(BlueprintPure, Category = "Character|Weapon")
	AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

	// --- Gravity ---

	UFUNCTION(BlueprintCallable, Category = "Character|Gravity")
	void SetGravityDirection(FVector NewDirection);

	UFUNCTION(BlueprintCallable, Category = "Character|Gravity")
	void ResetGravity();

	// --- Fall / Landing ---

	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall", meta=(ToolTip="현재 공중에서 낙하 중인지 (점프 상승 포함). AnimBP에서 읽기"))
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall", meta=(ToolTip="낙하 속도 (cm/s, 양수). 상승 중이면 0. AnimBP 블렌딩에 사용"))
	float CurrentFallSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Fall", meta=(ToolTip="이 속도(cm/s) 이상으로 착지하면 하드 랜딩 판정"))
	float HardLandingSpeedThreshold = 600.f;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall", meta=(ToolTip="하드 랜딩 직후 true. ResetHardLanding() 호출 전까지 유지"))
	bool bIsHardLanding = false;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character|Fall")
	void OnLanding(bool bHardLanding);
	virtual void OnLanding_Implementation(bool bHardLanding);

	UFUNCTION(BlueprintCallable, Category = "Character|Fall")
	void ResetHardLanding();

protected:
	virtual void Landed(const FHitResult& Hit) override;
};
