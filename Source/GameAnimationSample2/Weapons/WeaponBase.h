// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.h"
#include "WeaponDataAsset.h"
#include "WeaponBase.generated.h"

class UNiagaraSystem;
class USphereComponent;
class AGrenadeBase;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta=(ToolTip="무기 스켈레탈 메시 컴포넌트"))
	USkeletalMeshComponent* WeaponMesh;

	// --- Pickup ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Pickup", meta=(ToolTip="픽업 감지용 구체 콜리전"))
	USphereComponent* PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Pickup", meta=(ToolTip="픽업 감지 반경 (cm). DataAsset에서 설정"))
	float PickupRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Pickup", meta=(ToolTip="true면 월드에 떨어진 상태 (줍기 가능)"))
	bool bIsDropped = true;

	// --- Data ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data", meta=(ToolTip="할당 시 BeginPlay에서 Stats·Audio·VFX 값을 덮어씀"))
	UWeaponDataAsset* WeaponData = nullptr;

	// --- Config ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config", meta=(ToolTip="발사 방식. Hitscan=즉시 레이캐스트, Projectile=투사체 스폰"))
	EWeaponFireMode FireMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config", meta=(ToolTip="자동 연사 여부. true면 누르는 동안 계속 발사"))
	bool bIsAutoFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config", meta=(ToolTip="발사 버튼 누르는 동안 곡사 궤도 미리보기 표시"))
	bool bShowArcTrajectory = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config", meta=(ToolTip="true=팰릿 수만큼 탄약 소비(샷건). false=발사 1회당 1발 소비(DMR 점사)"))
	bool bAmmoPerPellet = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Config", meta=(ToolTip="소켓 기준 무기 메시 회전 오프셋. DataAsset에서 설정"))
	FRotator WeaponMeshRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Config", meta=(ToolTip="무기 메시 스케일. DataAsset에서 설정"))
	FVector WeaponMeshScale = FVector::OneVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Config", meta=(ToolTip="true면 소켓 기준 자동 정렬. DataAsset에서 설정"))
	bool bUseLeftHandGrip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Config", meta=(ToolTip="그립 정렬 후 추가 위치 오프셋. DataAsset에서 설정"))
	FVector WeaponMeshLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config", meta=(ToolTip="플레이어 발사 시 카메라 트레이스 시작점 전방 오프셋 (cm). 엄폐물 클리핑 방지용"))
	float TraceStartOffset = 30.f;

	// --- 소리 지각 (AI 청각) ---
	// 총을 쏠 때만 소음을 낸다. 적 AI의 Hearing 감각이 이 이벤트를 듣고 위치를 조사한다.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Perception", meta=(ClampMin="0", ToolTip="발사 소음이 퍼지는 거리 (cm). 0이면 소음을 내지 않는다. DataAsset에서 설정"))
	float FireNoiseRange = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Perception", meta=(ClampMin="0", ToolTip="발사 소음 크기 배율. 적 AI 청각 감지 범위에 곱해진다. DataAsset에서 설정"))
	float FireNoiseLoudness = 1.f;

	// --- 히트마커 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|HUD", meta=(ToolTip="이 문자열이 들어간 본에 맞으면 헤드샷으로 친다 (대소문자 무시)"))
	FString HeadBoneKeyword = TEXT("head");

	// 맞은 대상이 적 캐릭터면 플레이어 HUD에 히트마커를 알린다
	void ReportHitToPlayer(const FHitResult& Hit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config", meta=(ToolTip="발사 트레이스 디버그 라인 표시 여부"))
	bool bDebugTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config", meta=(ToolTip="소켓(GripStart·GripLeft·Root) 위치 디버그 드로 표시 여부"))
	bool bDebugGrip = false;

	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats", meta=(ToolTip="발사 1회당 피해량"))
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats", meta=(ToolTip="초당 발사 횟수 (rounds per second)"))
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats", meta=(ToolTip="히트스캔 최대 사거리 (cm)"))
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats", meta=(ToolTip="장전 소요 시간 (초)"))
	float ReloadTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats", meta=(ToolTip="발사 1회당 탄알 수. 1=단발, >1=샷건"))
	int32 PelletCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats", meta=(ToolTip="탄알 퍼짐 반각 (도). PelletCount > 1일 때 유효"))
	float PelletSpreadAngle = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats", meta=(ToolTip="팰릿 간 발사 간격 (초). 0=동시(샷건), >0=순차(점사)"))
	float PelletFireInterval = 0.f;

	// --- Ammo ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo", meta=(ToolTip="탄창 용량 (발)"))
	int32 MagSize;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo", meta=(ToolTip="현재 탄창에 남은 탄약 수"))
	int32 CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo", meta=(ToolTip="예비 탄약 수"))
	int32 ReserveAmmo;

	// --- Projectile ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile", meta=(ToolTip="Projectile 모드에서 스폰할 투사체 클래스. DataAsset에서 설정"))
	TSubclassOf<AProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile", meta=(ToolTip="Hitscan 모드에서 시각 효과용으로 스폰할 투사체 클래스 (충돌·피해 없음)"))
	TSubclassOf<AProjectileBase> CosmeticProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile", meta=(ToolTip="설정 시 Projectile 모드에서 ProjectileClass 대신 이 수류탄(AGrenadeBase)을 발사 — 유탄 발사기. DataAsset에서 설정"))
	TSubclassOf<AGrenadeBase> GrenadeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile", meta=(ToolTip="투사체 속도 덮어쓰기 (cm/s). 0이면 투사체 기본값 사용"))
	float ProjectileSpeedOverride = 0.f;

	// --- Spread ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Spread", meta=(ToolTip="발사 1회당 추가되는 크로스헤어 퍼짐량"))
	float SpreadPerShot = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Spread", meta=(ToolTip="장전 시작 시 추가되는 크로스헤어 퍼짐량"))
	float SpreadReloading = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Spread", meta=(ToolTip="초당 크로스헤어 퍼짐 회복량"))
	float SpreadRecoverySpeed = 15.f;

	// --- Recoil ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil", meta=(ToolTip="발사 1회당 카메라 위로 올라가는 각도 (도)"))
	float RecoilPitch = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil", meta=(ToolTip="발사 1회당 좌우 랜덤 흔들림 최대 각도 (도)"))
	float RecoilYawRange = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil", meta=(ToolTip="반동 최대 누적 각도 (도)"))
	float MaxRecoilPitch = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil", meta=(ToolTip="초당 반동 회복 속도 (도/초)"))
	float RecoilRecoverySpeed = 10.f;

	// 발사(반동) 몽타주는 캐릭터 공통이라 APlayerCharacter::FireMontage(MikaData)로 이동됨.

	// --- Audio ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Audio", meta=(ToolTip="발사 시 재생할 사운드 에셋"))
	USoundBase* FireSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Audio", meta=(ToolTip="잔탄 없음·장전 중·행동 불가 상태에서 발사 시도 시 재생할 사운드"))
	USoundBase* DryFireSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Audio", meta=(ToolTip="재장전 시작 시 재생할 사운드. DataAsset에서 설정"))
	USoundBase* ReloadSound = nullptr;

	// --- VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX", meta=(ToolTip="히트스캔 탄착점에 스폰할 나이아가라 시스템"))
	UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX", meta=(ToolTip="발사 시 총구에 스폰할 나이아가라 시스템 (머즐 플래시)"))
	UNiagaraSystem* MuzzleVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX", meta=(ToolTip="머즐 VFX 스케일"))
	float MuzzleVFXScale = 1.0f;

	// --- Internal state ---

	bool bIsReloading;
	bool bFireCooldown = false;
	FTimerHandle AutoFireTimerHandle;
	FTimerHandle SemiFireTimerHandle;
	FTimerHandle ReloadTimerHandle;
	FTimerHandle PelletFireTimerHandle;

	int32 CurrentPelletShot = 0;

	FTransform BaseRelativeTransform;

	void HitscanFire();
	void ProjectileFire();
	void GrenadeFire();	// 유탄 발사기: AGrenadeBase를 스폰해 조준 방향으로 Launch
	void FinishReload();
	void ResetSemiFireCooldown();
	void FireNextPelletInBurst();
	void AlignWeaponToLeftHand();

private:
	FTimerHandle PickupEnableTimerHandle;

	UFUNCTION()
	void EnablePickupSphere();

	UFUNCTION()
	void OnPickupSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPickupSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	// --- Pickup ---

	UFUNCTION(BlueprintCallable, Category = "Weapon|Pickup")
	void SetDropped(bool bDropped);

	UFUNCTION(BlueprintPure, Category = "Weapon|Pickup")
	bool IsDropped() const { return bIsDropped; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Pickup")
	USoundBase* GetPickupSound() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Pickup")
	FText GetWeaponDisplayName() const;

	FName LeftHandGripStartSocketName = TEXT("LeftHandGripStart");
	FName LeftHandGripSocketName      = TEXT("LeftHandGrip");
	FName GripSocketName              = TEXT("GripSocket");
	FName MuzzleSocketName            = TEXT("MuzzleSocket");
	FName LeftHandSocketName          = TEXT("WeaponSocketLeft");

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RefreshMeshTransform();

	UFUNCTION(BlueprintPure, Category = "Weapon|Animation")
	FTransform GetLeftHandGripTransform() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsAutoFire() const { return bIsAutoFire; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool ShowsArcTrajectory() const { return bShowArcTrajectory; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FVector GetMuzzleLocation() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetProjectileSpeedForTrajectory() const { return FMath::Max(ProjectileSpeedOverride, 100.f); }

	// --- Fire ---

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();

	// --- Reload ---

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	int32 GetReserveAmmo() const { return ReserveAmmo; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	bool IsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	float GetReloadTime() const { return ReloadTime; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	int32 GetMagSize() const { return MagSize; }

	// AI 무한 탄약용. 플레이어 탄약 획득 로직이 생기면 그쪽도 이 함수를 쓴다
	UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
	void SetReserveAmmo(int32 NewReserve) { ReserveAmmo = FMath::Max(NewReserve, 0); }

	// 지금 방아쇠를 당기면 실제로 탄이 나가는가 — 단발 무기를 AI가 연사 속도에 맞춰 다시 당길 때 사용.
	// (StartFire를 쿨다운 중에 부르면 빈 총 소리가 나므로 미리 확인한다)
	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	bool IsFireReady() const { return CanFire() && !bFireCooldown; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Recoil")
	float GetRecoilPitch() const { return RecoilPitch; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Recoil")
	float GetRecoilYawRange() const { return RecoilYawRange; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Recoil")
	float GetMaxRecoilPitch() const { return MaxRecoilPitch; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Recoil")
	float GetRecoilRecoverySpeed() const { return RecoilRecoverySpeed; }

	// --- Events (override in Blueprint for VFX/SFX) ---

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	void OnFire(const FHitResult& HitResult);
	virtual void OnFire_Implementation(const FHitResult& HitResult);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	void OnReloadStart();
	virtual void OnReloadStart_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	void OnReloadFinish();
	virtual void OnReloadFinish_Implementation();
};
