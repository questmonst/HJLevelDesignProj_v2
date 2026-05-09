// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TraversalComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GAMEANIMATIONSAMPLE2_API UTraversalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTraversalComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Enable ---

    /** false면 TryTraversal() 호출을 무시함 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
    bool bEnabled = true;

    // --- Detection ---

    /** 전방 장애물 감지 거리 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float ForwardTraceDistance = 80.f;

    /** 트래버설을 감지하는 최소 장애물 높이 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float MinObstacleHeight = 20.f;

    /** 트래버설 감지 최대 장애물 높이 (cm): 이 이상이면 트래버설 불가 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float MaxObstacleHeight = 285.f;

    /** 착지 위치를 장애물 상단에서 앞쪽으로 얼마나 이동할지 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float LandingForwardOffset = 40.f;

    /** 착지 목표 Z를 장애물 상단 기준으로 얼마나 낮출지 (cm, 음수 = 아래) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection")
    float LandingHeightOffset = -30.f;

    // --- Jump ---

    /** 포물선 최고점의 추가 높이 (cm). 목표-현재 Z차이 위에 더 올라가는 높이 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Jump")
    float JumpArcHeight = 80.f;

    /** 연속 발동 방지 쿨다운 (초). 공중에서도 이 간격으로 재발동 가능 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Jump")
    float JumpCooldown = 0.5f;

    /** 현재 전방에 트래버설 가능한 장애물이 있는지 (HUD 등에서 참조) */
    UPROPERTY(BlueprintReadOnly, Category = "Traversal")
    bool bCanTraverse = false;

public:
    /**
     * Jump 입력 시 호출 (공중도 가능).
     * 전방 장애물이 감지되면 목표 지점으로 포물선 점프하고 true 반환.
     * 감지 실패 또는 쿨다운 중이면 false 반환 (일반 Jump 진행).
     */
    bool TryTraversal();

private:
    ACharacter* OwnerCharacter = nullptr;
    FTimerHandle CooldownTimerHandle;
    bool bOnCooldown = false;

    /** 트레이스로 장애물을 감지하고 착지 목표 위치를 계산. */
    bool DetectTraversal(FVector& OutLandingPos) const;
};
