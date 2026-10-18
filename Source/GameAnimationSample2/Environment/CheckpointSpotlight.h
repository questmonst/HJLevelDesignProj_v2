// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataAsset.h"
#include "Subsystems/WorldSubsystem.h"
#include "CheckpointSpotlight.generated.h"

class UBoxComponent;
class USpotLightComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class USoundBase;
class ACheckpointSpotlight;

// 체크포인트 조명·빛줄기·바닥 링·소리 (DA_CheckpointSpotlight). 여러 체크포인트가 공유한다.
// 꺼진 체크포인트(아직 안 밟음)와 현재 체크포인트(리스폰 지점)의 모습을 따로 정한다
UCLASS(BlueprintType)
class GAMEANIMATIONSAMPLE2_API UCheckpointData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color", meta=(ToolTip="아직 밟지 않은 체크포인트 색 (조명·빛줄기·바닥 링 공통)"))
	FLinearColor InactiveColor = FLinearColor(0.45f, 0.55f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color", meta=(ToolTip="현재 체크포인트(리스폰 지점) 색"))
	FLinearColor ActiveColor = FLinearColor(1.f, 0.9f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color", meta=(ClampMin="0", ToolTip="꺼진 쪽↔켜진 쪽으로 바뀌는 시간(초)"))
	float BlendTime = 0.4f;

	// --- 실제 조명 (바닥에 떨어지는 빛) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta=(ClampMin="0", ToolTip="아직 밟지 않은 체크포인트 조명 밝기. 0이면 꺼둔다"))
	float InactiveIntensity = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta=(ClampMin="0", ToolTip="현재 체크포인트 조명 밝기"))
	float ActiveIntensity = 20000.f;

	// --- 빛줄기 (조명과 무관하게 스스로 빛나는 반투명 원뿔 — 밝은 레벨에서도 보인다) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam", meta=(ToolTip="빛줄기 머티리얼 (M_CheckpointBeam). Color 벡터·Intensity 스칼라 파라미터를 쓴다. 비우면 빛줄기 없음"))
	UMaterialInterface* BeamMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam", meta=(ClampMin="0", ToolTip="꺼진 체크포인트 빛줄기 밝기 — 멀리서 \"저기 있다\" 정도로 희미하게"))
	float InactiveBeamIntensity = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam", meta=(ClampMin="0", ToolTip="현재 체크포인트 빛줄기 밝기"))
	float ActiveBeamIntensity = 0.8f;

	// --- 바닥 링 (리스폰 지점 표시) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ring", meta=(ToolTip="바닥 링 머티리얼 (M_CheckpointRing). Color·Intensity 파라미터. 비우면 링 없음"))
	UMaterialInterface* RingMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ring", meta=(ClampMin="0", ToolTip="바닥 링 반지름(cm)"))
	float RingRadius = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ring", meta=(ClampMin="0", ToolTip="꺼진 체크포인트 링 밝기"))
	float InactiveRingIntensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ring", meta=(ClampMin="0", ToolTip="현재 체크포인트 링 밝기"))
	float ActiveRingIntensity = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta=(ToolTip="체크포인트가 켜질 때 소리"))
	USoundBase* ActivateSound = nullptr;
};

// 체크포인트 겸 리스폰 스포트라이트 — 플레이어가 조명 아래(ActivateZone)에 들어오면 현재 체크포인트가 된다.
// 낙사하거나 죽어서 다시 시작하면 마지막 체크포인트의 화살표 위치·방향으로 돌아온다.
//
// 보이는 것: 실제 스포트라이트(바닥에 떨어지는 빛) + 빛줄기 원뿔 + 바닥 링.
// 빛줄기는 스포트라이트 높이·각도에 맞춰 자동으로 늘어난다 — 조명만 옮기면 된다.
// 처음 켜질 때 SignalTargets에 신호를 보낸다 (스포너 시작 등).
UCLASS(Blueprintable)
class GAMEANIMATIONSAMPLE2_API ACheckpointSpotlight : public AActor
{
	GENERATED_BODY()

public:
	ACheckpointSpotlight();

	virtual void Tick(float DeltaTime) override;

	// 현재 체크포인트가 되거나(true) 다른 곳에 넘겨줄 때(false) — 관리자가 부른다
	void SetActiveCheckpoint(bool bActive);

	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	int32 GetCheckpointOrder() const { return CheckpointOrder; }

	// 리스폰 위치·방향 (액터 위치, 화살표 방향)
	FTransform GetRespawnTransform() const { return GetActorTransform(); }

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta=(ToolTip="플레이어가 들어오면 체크포인트가 켜지는 영역"))
	UBoxComponent* ActivateZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta=(ToolTip="위에서 리스폰 지점을 비추는 조명. 높이(Z)·Outer Cone Angle을 바꾸면 빛줄기도 따라간다"))
	USpotLightComponent* Spotlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta=(ToolTip="빛줄기 원뿔 (자동 크기)"))
	UStaticMeshComponent* BeamMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta=(ToolTip="바닥 링 (자동 크기)"))
	UStaticMeshComponent* FloorRing;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UArrowComponent* RespawnArrow;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta=(ToolTip="모습·소리 DA (DA_CheckpointSpotlight)"))
	UCheckpointData* CheckpointData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta=(ToolTip="진행 순서. bOnlyForward면 이 번호가 현재보다 크거나 같을 때만 켜진다 (되돌아가도 리스폰 지점이 뒤로 가지 않게)"))
	int32 CheckpointOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta=(ToolTip="true면 앞 번호 체크포인트로 되돌아가도 리스폰 지점이 바뀌지 않는다"))
	bool bOnlyForward = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta=(ToolTip="처음 켜질 때 신호를 보낼 액터 (스포너·문·스위치). 레벨에서 스포이트로 지정"))
	TArray<TObjectPtr<AActor>> SignalTargets;

	bool  bIsActive    = false;
	bool  bSignalSent  = false;
	float ActiveAlpha  = 0.f;   // 0 꺼짐 ~ 1 켜짐 (색·밝기를 이 비율로 섞는다)

	UPROPERTY(Transient) UMaterialInstanceDynamic* BeamMID = nullptr;
	UPROPERTY(Transient) UMaterialInstanceDynamic* RingMID = nullptr;

	const UCheckpointData& GetData() const;
	void FitVisuals();          // 빛줄기·링 크기를 조명 높이·각도·DA 반지름에 맞춤
	void CreateMaterials();
	void ApplyVisuals();        // ActiveAlpha에 맞춰 조명·빛줄기·링 갱신

	UFUNCTION()
	void HandleZoneBegin(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);

	friend class UCheckpointSubsystem;
};

DECLARE_MULTICAST_DELEGATE(FOnPlayerRespawnedSignature);

// 현재 체크포인트 관리 — 리스폰 위치를 알려주고, 플레이어가 죽었다 다시 시작하면 방송한다
UCLASS()
class GAMEANIMATIONSAMPLE2_API UCheckpointSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 체크포인트가 플레이어를 감지했을 때. 순서 규칙을 통과하면 현재 체크포인트로 바꾼다
	void TryActivate(ACheckpointSpotlight* Checkpoint);

	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	ACheckpointSpotlight* GetCurrentCheckpoint() const { return Current.Get(); }

	// 현재 체크포인트가 켜진 시각 — 이보다 뒤에 시작한 전투·트리거는 다시 시작할 때 되돌린다. 없으면 -1
	float GetCheckpointTime() const { return CheckpointTime; }

	// 리스폰 위치·방향 — 마지막 체크포인트, 없으면 플레이어 스타트. 찾지 못하면 false.
	// 체크포인트 원점은 바닥이라 캐릭터 캡슐 반 높이만큼 올려 준다 (낙사 구역·사망 재시작 공용)
	bool FindRespawnPoint(const ACharacter* Character, AController* Controller, FVector& OutLocation, FRotator& OutRotation) const;

	// 플레이어가 죽었다가 다시 시작했을 때 (스포너·트리거가 듣고 상태를 되돌린다)
	void NotifyPlayerRespawned() { OnPlayerRespawned.Broadcast(); }
	FOnPlayerRespawnedSignature OnPlayerRespawned;

private:
	TWeakObjectPtr<ACheckpointSpotlight> Current;
	float CheckpointTime = -1.f;
};
