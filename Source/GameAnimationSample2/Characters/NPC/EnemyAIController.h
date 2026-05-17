// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

UCLASS()
class GAMEANIMATIONSAMPLE2_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // --- Perception ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception", meta=(ToolTip="AI 감지 컴포넌트"))
    UAIPerceptionComponent* AIPerception;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception", meta=(ToolTip="시야 반경 (cm)"))
    float SightRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception", meta=(ToolTip="시야 상실 반경 (cm). SightRadius보다 커야 함"))
    float LoseSightRadius = 2500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception", meta=(ToolTip="주변 시야각 (도, 양쪽 합산)"))
    float PeripheralVisionAngleDeg = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception", meta=(ToolTip="청각 반경 (cm)"))
    float HearingRange = 1200.f;

public:
    virtual FGenericTeamId GetGenericTeamId() const override;

private:
    UAISenseConfig_Sight*   SightConfig   = nullptr;
    UAISenseConfig_Hearing* HearingConfig = nullptr;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
