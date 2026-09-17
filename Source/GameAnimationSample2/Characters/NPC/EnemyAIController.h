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
    virtual void Tick(float DeltaSeconds) override;

    // 피격 시 AEnemyCharacter가 호출 — 공격자를 못 보고 있으면 그 위치를 TargetLocation에 써서
    // BT가 그쪽을 바라보게 한다. 공격자를 직접 타겟으로 삼지 않는 이유: 시야 확인은 Perception의 몫
    void NotifyDamagedBy(AActor* Attacker);

private:
    // 경계 상태에서 스폰 지점 이탈·장기간 미발견 시 타겟을 잊고 순찰로 복귀시킨다
    void UpdateForget();
    void ForgetTarget();

    // 타겟 정보(시야·피격)를 마지막으로 얻은 시각. 잊기 판정 기준
    float LastTargetInfoTime = 0.f;

    UAISenseConfig_Sight*   SightConfig   = nullptr;
    UAISenseConfig_Hearing* HearingConfig = nullptr;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
