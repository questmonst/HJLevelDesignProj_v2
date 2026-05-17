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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", meta=(ToolTip="false면 TryTraversal() 호출을 무시함"))
    bool bEnabled = true;

    // --- Detection ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection", meta=(ToolTip="전방 장애물 감지 거리 (cm)"))
    float ForwardTraceDistance = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection", meta=(ToolTip="트래버설을 감지하는 최소 장애물 높이 (cm)"))
    float MinObstacleHeight = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection", meta=(ToolTip="트래버설 감지 최대 장애물 높이 (cm). 이 이상이면 트래버설 불가"))
    float MaxObstacleHeight = 285.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection", meta=(ToolTip="착지 위치를 장애물 상단에서 앞쪽으로 얼마나 이동할지 (cm)"))
    float LandingForwardOffset = 40.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Detection", meta=(ToolTip="착지 목표 Z를 장애물 상단 기준으로 얼마나 낮출지 (cm). 음수=아래"))
    float LandingHeightOffset = -30.f;

    // --- Jump ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Jump", meta=(ToolTip="포물선 최고점의 추가 높이 (cm). 목표-현재 Z차이 위에 더 올라가는 높이"))
    float JumpArcHeight = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Jump", meta=(ToolTip="연속 발동 방지 쿨다운 (초). 공중에서도 이 간격으로 재발동 가능"))
    float JumpCooldown = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Traversal", meta=(ToolTip="현재 전방에 트래버설 가능한 장애물이 있는지 (HUD 등에서 참조)"))
    bool bCanTraverse = false;

public:
    bool TryTraversal();

private:
    ACharacter* OwnerCharacter = nullptr;
    FTimerHandle CooldownTimerHandle;
    bool bOnCooldown = false;

    bool DetectTraversal(FVector& OutLandingPos) const;
};
