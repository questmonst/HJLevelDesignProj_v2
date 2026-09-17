// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception", meta=(ClampMin="0.1", ToolTip="피격으로 즉시 교전에 들어간 뒤, 이 시간(초) 안에 시야 감각이 공격자를 못 잡으면 교전 해제"))
    float DamageEngageGraceTime = 1.f;

public:
    virtual FGenericTeamId GetGenericTeamId() const override;
    virtual void Tick(float DeltaSeconds) override;


private:
    // 경계 상태에서 스폰 지점 이탈·장기간 미발견 시 타겟을 잊고 순찰로 복귀시킨다
    void UpdateForget();
    void ForgetTarget();

    // 피격으로 켠 bCanSeeTarget을 시야 감각이 이어받았는지 확인.
    // Damage 감각은 일회성 이벤트라 "감지 해제" 알림이 오지 않으므로 직접 정리해야 한다
    void UpdateDamageEngage();
    bool IsSightSensing(AActor* Actor) const;

    bool  bDamageEngaged = false;
    float DamageEngageTime = 0.f;

    // 타겟 정보(시야·피격)를 마지막으로 얻은 시각. 잊기 판정 기준
    float LastTargetInfoTime = 0.f;

    UAISenseConfig_Sight*   SightConfig   = nullptr;
    UAISenseConfig_Hearing* HearingConfig = nullptr;
    UAISenseConfig_Damage*  DamageConfig  = nullptr;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
