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
class UAnimSequenceBase;
class UMotionWarpingComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrenadeThrowReadyChangedSignature, bool, bReady);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement", meta=(ToolTip="앉아서 이동하는 속도 (cm/s). CharacterMovement.MaxWalkSpeedCrouched에 적용"))
	float CrouchWalkSpeed = 200.f;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="조준 중 카메라를 기본 위치에서 더 오른쪽으로 옮기는 거리(cm, 음수=왼쪽). 엄폐 좌우 이동에 더해짐"))
	float AimSocketOffsetRight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="조준 중 카메라를 위로 올리는 거리(cm, 음수=아래). 스프링암 SocketOffset.Z"))
	float AimSocketOffsetUp = 0.f;

	// DefaultSocketOffsetY를 스프링암·엄폐 기준값에 적용. 자식이 DataAsset 값을 복사한 뒤 다시 호출해야 DA 값이 먹는다
	void ApplyDefaultSocketOffset();

	void UpdateCoverPeek(float DeltaTime);
	void UpdateAimSpinePitch(float DeltaTime);
	void UpdateCrosshairSpread(float DeltaTime);
	void UpdateRecoil(float DeltaTime);

	// --- Crouch Camera ---
	// 앉기 시 시점 보정. 스프링암 TargetOffset.Z(월드 상하)만 사용해 다른 전투 카메라와 독립.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="앉기 시 카메라를 위로 올리는 높이 (cm, 월드 기준)"))
	float CrouchCameraZOffset = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="true=앉기 카메라 높이 보간, false=즉시 전환"))
	bool bSmoothCrouchCamera = true;

	// --- Camera (메시 따라가기) ---
	// 점프·펀치 모션은 골반이 캡슐 위로 크게 올라가 몸이 화면 밖으로 나간다. 켜면 카메라 피벗이 그만큼 같이 올라간다.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="공중 포즈일 때 카메라가 보이는 몸(기준 본)을 따라가게 할지. MikaData에서 설정"))
	bool bCameraFollowMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="카메라가 따라갈 기준 본. 보통 골반"))
	FName CameraFollowMeshBone = TEXT("valvebiped_bip01_pelvis");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ClampMin="0", ToolTip="메시 따라가기 보간 속도. 클수록 즉시 따라감"))
	float CameraFollowMeshInterpSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ClampMin="0", ToolTip="메시 따라가기 최대 보정 높이(cm). 모션이 튀어도 이 이상은 안 올라감"))
	float CameraFollowMeshMaxOffset = 200.f;

	float CameraFollowMeshZ   = 0.f;   // 현재 적용 중인 보정 높이
	float CameraFollowBaseGap = 0.f;   // 평상시 기준 본과 캡슐 중심의 높이 차 (BeginPlay에서 측정)
	float CrouchCameraZ       = 0.f;   // 앉기 보정 높이 (메시 보정과 합산해 TargetOffset.Z에 적용)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera", meta=(ToolTip="앉기 카메라 보간 속도. bSmoothCrouchCamera=true일 때만 사용"))
	float CrouchCameraInterpSpeed = 10.f;

	void UpdateCrouchCamera(float DeltaTime);

	// 앉기/서기 전환 시 카메라 높이 적용 (엔진이 Crouch/UnCrouch에서 호출)
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	float SpreadAdditive   = 0.f;
	float RecoilPitchAccum = 0.f;

private:
	float NormalSocketOffsetY = 0.f;
	float NormalSocketOffsetZ = 0.f;   // BP에 설정된 기본 SocketOffset.Z (조준 해제 시 복귀값)

protected:

	// --- Grenade ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade", meta=(ToolTip="스폰할 수류탄 클래스"))
	TSubclassOf<AGrenadeBase> GrenadeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade", meta=(ToolTip="수류탄 초기 투사 속도 (cm/s)"))
	float GrenadeThrowSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade", meta=(ToolTip="조준선(궤적) 시작점 = 실제 발사 위치. 카메라 기준 오프셋 (X=앞, Y=오른쪽, Z=위, cm). MikaData에서 설정"))
	FVector GrenadeLaunchOffset = FVector(50.f, 0.f, 0.f);

	// 조준선과 실제 발사가 같은 시작점·속도를 쓰도록 한 곳에서 계산 — 둘이 어긋나면 조준선을 믿을 수 없다
	bool GetGrenadeLaunchParams(FVector& OutStart, FVector& OutVelocity) const;

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

	UPROPERTY(BlueprintReadOnly, Category = "Character|Grenade", meta=(ToolTip="수류탄을 손에 들고 조준 중(던지기 버튼 누름). ABP에서 준비 자세 분기에 사용"))
	bool bIsPreparingThrow = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Grenade", meta=(ToolTip="던지기 몽타주 재생 중. ABP에서 던지기 동작 분기에 사용"))
	bool bIsThrowingGrenade = false;

	FTimerHandle GrenadeThrowEndTimerHandle;
	FTimerHandle GrenadeReleaseFallbackTimerHandle;   // 노티파이가 없을 때 몽타주 끝에 강제로 놓기
	void EndGrenadeThrowAnim();

	// 수류탄 때문에 조준 상태로 들어갔는지 — 원래 조준 중이었다면 던진 뒤에도 조준을 유지한다
	bool bAimStartedByGrenade = false;
	void EndGrenadeAim();

public:
	// 손에 든 수류탄을 지금 조준 방향으로 던진다. 던지기 몽타주의 AnimNotify_GrenadeRelease가 호출
	void LaunchHeldGrenade();

	UPROPERTY(BlueprintAssignable, Category = "Character|Grenade", meta=(ToolTip="손에 든 수류탄이 던질 수 있게 되면 true, 던지거나 취소하면 false. UI 표시용"))
	FOnGrenadeThrowReadyChangedSignature OnGrenadeThrowReadyChanged;

protected:
	// 손에 든 수류탄의 준비 완료 알림을 받아 OnGrenadeThrowReadyChanged(true) 방송
	void OnHeldGrenadeReady();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Animation", meta=(ToolTip="앉은 상태 발사 몽타주. 비어 있으면 FireMontage를 쓴다"))
	UAnimMontage* FireMontageCrouch = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Animation", meta=(ToolTip="수류탄 준비(손에 들고 조준) 몽타주. MikaData에서 설정"))
	UAnimMontage* GrenadePrepareMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Animation", meta=(ToolTip="수류탄 던지기 몽타주. MikaData에서 설정"))
	UAnimMontage* GrenadeThrowMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Animation", meta=(ToolTip="재장전 몽타주(UpperBody 슬롯). MikaData에서 설정"))
	UAnimMontage* ReloadMontage = nullptr;

	// --- 장전 중 왼손 부착 ---
	// 장전 모션에서는 총을 왼손이 들고 오른손이 탄창을 다룬다. 무기의 LeftHandGrip 소켓이
	// 캐릭터의 왼손 소켓에 오도록 무기를 옮겼다가, 장전이 끝나면 원래 손으로 되돌린다.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon", meta=(ToolTip="true면 장전하는 동안 무기를 왼손 소켓으로 옮긴다. MikaData에서 설정"))
	bool bAttachWeaponToLeftHandOnReload = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon", meta=(ToolTip="장전 중 무기를 붙일 캐릭터 메시 소켓 (미카 왼손). MikaData에서 설정"))
	FName ReloadLeftHandSocket = TEXT("LeftHand");

	FTimerHandle ReloadAttachTimerHandle;
	bool bWeaponInLeftHand = false;

	void AttachWeaponToLeftHand();
	void RestoreWeaponToRightHand();

	// 맨손 동작(펀치·수류탄) 동안 총을 숨긴다 — 애니가 총을 쥔 손과 겹쳐 보이지 않도록
	void SetCurrentWeaponHidden(bool bHideWeapon);

	// 몽타주를 재생하고 실제 재생 시간(초)을 돌려준다. PlayAnimMontage는 Rate Scale을 반영하지 않은
	// 원본 길이를 돌려주므로 보정 — 펀치 대시·수류탄 던지기처럼 애니 길이에 로직을 맞출 때 공용으로 사용
	float PlayMontageForDuration(UAnimMontage* Montage);

	// 실제로 재생 중인 발사 몽타주. 사격 도중 앉기/서기가 바뀌어도
	// StopFire가 엉뚱한 몽타주를 멈추지 않도록 시작 시점의 것을 들고 있는다.
	UPROPERTY(Transient)
	UAnimMontage* ActiveFireMontage = nullptr;

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

	// WeaponBase::Fire가 탄 1발을 쏠 때마다 호출 — 반동 적용 + 마지막 탄이면 반동 몽타주 정지
	// (연사 중 탄창이 비면 버튼을 떼기 전까지 몽타주가 계속 돌던 문제)
	void OnWeaponShotFired();

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

	// 현재 자세에 맞는 발사 몽타주를 고른다 (앉기 전용이 없으면 기본값)
	UFUNCTION(BlueprintPure, Category = "Character|Animation")
	UAnimMontage* SelectFireMontage() const;

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

	// ABP 점프 상태 머신 분기용 — 착지 모션이 끝까지 재생되도록 착지 후에도 잠시 유지한다
	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall", meta=(ToolTip="공중 포즈를 쓸지 (낙하 중이거나 착지 후 LandPoseHoldTime 이내). AnimBP 점프 분기에서 읽기"))
	bool bIsInAirPose = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Fall", meta=(ClampMin="0", ToolTip="착지 후 공중 포즈를 유지하는 시간(초). 착지 모션 길이에 맞춘다. MikaData에서 설정"))
	float LandPoseHoldTime = 0.47f;

	// bIsInAirPose를 부드럽게 만든 값 — ABP의 Layered blend per bone(하체만 점프) 가중치에 그대로 연결
	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall", meta=(ToolTip="공중 포즈 가중치 0~1 (bIsInAirPose를 보간). AnimBP 하체 레이어 Blend Weights에 사용"))
	float AirPoseAlpha = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Fall", meta=(ClampMin="0", ToolTip="공중 포즈 가중치 보간 속도. 클수록 빠르게 전환 (0이면 즉시). MikaData에서 설정"))
	float AirPoseBlendSpeed = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Fall", meta=(ClampMin="0.1", ToolTip="점프 모션 재생 속도 배율. ABP AirLoco 시퀀스 플레이어 Play Rate에 바인딩. MikaData에서 설정"))
	float JumpAnimPlayRate = 1.f;

	// 착지 유지 시간을 클립 길이에서 자동으로 구한다 — 클립을 손봐도 C++ 수치를 다시 맞출 필요가 없다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Fall", meta=(ToolTip="ABP Land 상태가 재생하는 착지 클립. 지정하면 착지 유지 시간을 길이에서 자동 계산 (비우면 LandPoseHoldTime 사용). MikaData에서 설정"))
	TObjectPtr<UAnimSequenceBase> LandAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Fall", meta=(ClampMin="0", ToolTip="착지 클립을 몇 초 지점부터 재생할지. ABP Land 상태 Start Position에 바인딩. MikaData에서 설정"))
	float LandAnimStartTime = 0.05f;

	// 착지 모션이 끝나는 시점 (재생 속도·시작 지점 반영). 이 시간이 지나면 바로 지상 로코모션으로 돌아간다
	UFUNCTION(BlueprintPure, Category = "Character|Fall")
	float GetLandHoldTime() const;

	// 착지 모션을 중간에 끊을 상황인지 (이동 입력·행동). 자식이 자기 행동을 더한다
	UFUNCTION(BlueprintPure, Category = "Character|Fall")
	virtual bool IsLandPoseInterrupted() const;

	// 착지 구간(예측 시작 ~ 착지 후 유지 시간)에서는 하체만이 아니라 전신으로 착지 모션을 보여준다
	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall", meta=(ToolTip="착지 포즈 가중치 0~1. AnimBP에서 '하체만 결과'와 '점프 상태 머신 전신'을 섞는 알파로 사용"))
	float LandPoseAlpha = 0.f;

	float LandedTime = -1000.f;   // 마지막 착지 시각

	// 착지 모션을 착지 직전에 시작하기 위한 예측 — 발밑으로 훑어 남은 거리 ÷ 낙하 속도
	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall", meta=(ToolTip="곧 착지함 (예상 착지까지 LandAnticipationTime 이내). AnimBP에서 Land 상태 전환 조건으로 사용"))
	bool bIsLandingSoon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Fall", meta=(ClampMin="0", ToolTip="착지 몇 초 전에 착지 모션을 시작할지. 0이면 예측 끔. MikaData에서 설정"))
	float LandAnticipationTime = 0.15f;

	void UpdateLandingPrediction();

	UPROPERTY(BlueprintReadOnly, Category = "Character|Fall", meta=(ToolTip="하드 랜딩 직후 true. ResetHardLanding() 호출 전까지 유지"))
	bool bIsHardLanding = false;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character|Fall")
	void OnLanding(bool bHardLanding);
	virtual void OnLanding_Implementation(bool bHardLanding);

	UFUNCTION(BlueprintCallable, Category = "Character|Fall")
	void ResetHardLanding();

protected:
	virtual void Landed(const FHitResult& Hit) override;

public:
	// 플레이어는 사망해도 Destroy하지 않는다 — HUD 위젯(크로스헤어 등)이 파괴된 캐릭터를 읽어 에러가 나고,
	// 체크포인트 재시작 시 같은 캐릭터를 되살려 쓰기 위해서다. 지금은 입력·사격·이동만 멈춘다.
	virtual void OnDeath_Implementation() override;
};
