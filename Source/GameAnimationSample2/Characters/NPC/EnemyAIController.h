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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    UAIPerceptionComponent* AIPerception;

    /** 시야 반경 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float SightRadius = 2000.f;

    /** 시야 상실 반경 (cm, SightRadius보다 커야 함) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float LoseSightRadius = 2500.f;

    /** 주변 시야각 (도, 양쪽 합산) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float PeripheralVisionAngleDeg = 60.f;

    /** 청각 반경 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float HearingRange = 1200.f;

private:
    UAISenseConfig_Sight*   SightConfig   = nullptr;
    UAISenseConfig_Hearing* HearingConfig = nullptr;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
