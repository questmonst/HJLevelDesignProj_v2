// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TraversalComponent.generated.h"

/**
 * 장애물 트래버설 타입.
 * 높이 기준:
 *   Hurdle  ~60cm  이하 — 낮은 장애물 넘기
 *   Vault   ~60-150cm   — 중간 장애물 빠르게 뛰어넘기
 *   Mantle  ~150-230cm  — 높은 장애물 매달려 올라가기
 *   Climb   ~230-280cm  — 매우 높은 장애물 (2.5m 기준 애니메이션)
 */
UENUM(BlueprintType)
enum class ETraversalType : uint8
{
    None   UMETA(DisplayName = "None"),
    Hurdle UMETA(DisplayName = "Hurdle"),
    Vault  UMETA(DisplayName = "Vault"),
    Mantle UMETA(DisplayName = "Mantle"),
    Climb  UMETA(DisplayName = "Climb"),
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GAMEANIMATIONSAMPLE2_API UTraversalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTraversalComponent();

protected:
    virtual void BeginPlay() override;

    // --- Detection ---

    /** 전방 장애물 감지 거리 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float ForwardTraceDistance = 80.f;

    /** 허들 최대 높이 (cm): 이 이하면 허들 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float HurdleMaxHeight = 60.f;

    /** 볼트 최대 높이 (cm): 이 이하면 볼트 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float VaultMaxHeight = 150.f;

    /** 맨틀 최대 높이 (cm): 이 이하면 맨틀, 이상이면 클라임 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float MantleMaxHeight = 230.f;

    /** 클라임 최대 높이 (cm): 이 이상이면 트래버설 불가 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float ClimbMaxHeight = 285.f;

    /** 트래버설을 감지하는 최소 장애물 높이 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float MinObstacleHeight = 20.f;

    /** 착지 위치를 장애물 상단에서 앞쪽으로 얼마나 이동할지 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float LandingForwardOffset = 40.f;

    // --- State ---

    UPROPERTY(BlueprintReadOnly, Category = "Traversal")
    bool bIsTraversing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Traversal")
    ETraversalType CurrentTraversalType = ETraversalType::None;

    /**
     * 트래버설 후 착지 위치 (MotionWarping "TraversalLanding" 타겟).
     * AnimBP의 MotionWarping 노드에서 이 이름으로 참조.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Traversal")
    FTransform WarpTargetTransform;

    /**
     * 가장자리(손이 닿는 지점) 위치 (MotionWarping "TraversalLedge" 타겟).
     * Mantle/Climb 애니메이션이 이 위치로 손을 뻗음.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Traversal")
    FTransform LedgeTransform;

public:
    /**
     * Jump 입력 시 호출.
     * 트래버설이 가능하면 시작하고 true 반환 (Jump 소비).
     * 불가능하면 false 반환 (일반 Jump 진행).
     */
    bool TryTraversal();

    /**
     * AnimNotify 또는 BlueprintCallable 로 트래버설 종료.
     * 캐릭터를 착지 위치로 이동하고 이동 모드를 복원.
     */
    UFUNCTION(BlueprintCallable, Category = "Traversal")
    void FinishTraversal();

    UFUNCTION(BlueprintPure, Category = "Traversal")
    bool IsTraversing() const { return bIsTraversing; }

    UFUNCTION(BlueprintPure, Category = "Traversal")
    ETraversalType GetTraversalType() const { return CurrentTraversalType; }

    UFUNCTION(BlueprintPure, Category = "Traversal")
    FTransform GetWarpTarget() const { return WarpTargetTransform; }

    UFUNCTION(BlueprintPure, Category = "Traversal")
    FTransform GetLedgeTransform() const { return LedgeTransform; }

protected:
    /**
     * 트래버설 시작 시 호출 — BP에서 오버라이드해 몽타주 재생.
     * @param Type        결정된 트래버설 타입
     * @param WarpTarget  착지 목표 트랜스폼 (TraversalLanding)
     * @param Ledge       가장자리 트랜스폼 (TraversalLedge)
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Traversal")
    void OnTraversalStart(ETraversalType Type, const FTransform& WarpTarget, const FTransform& Ledge);
    virtual void OnTraversalStart_Implementation(ETraversalType Type, const FTransform& WarpTarget, const FTransform& Ledge);

private:
    ACharacter* OwnerCharacter = nullptr;

    /** 트레이스를 수행하고 트래버설 타입 및 목표 트랜스폼을 계산. */
    bool DetectTraversal(ETraversalType& OutType, FTransform& OutWarpTarget, FTransform& OutLedge) const;
};
