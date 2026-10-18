// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallResetTrigger.generated.h"

class UBoxComponent;
class AFallResetTarget;

/**
 * 낙사 구역 (리셋 트리거).
 * 이 액터의 콜리전에 플레이어가 닿으면 화면이 어두워졌다가 리스폰 지점으로 순간이동한다.
 *
 * 리스폰 지점 우선순위:
 *   1. ResetTarget이 지정돼 있으면 그곳 (특정 구간 전용 리셋)
 *   2. 아니면 마지막으로 켠 체크포인트(ACheckpointSpotlight)
 *   3. 체크포인트가 없으면 플레이어 스타트
 *
 * 사용법: 낙사 구역 아래에 배치하고 박스 크기를 맞춘다. 보통은 ResetTarget을 비워 두고 체크포인트를 따른다
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API AFallResetTrigger : public AActor
{
    GENERATED_BODY()

public:
    AFallResetTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FallReset", meta=(ToolTip="낙사 감지 박스 컴포넌트"))
    UBoxComponent* TriggerBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallReset", meta=(ToolTip="이 구역 전용 리스폰 지점. 비우면 마지막 체크포인트(없으면 플레이어 스타트)"))
    AFallResetTarget* ResetTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallReset", meta=(ToolTip="텔레포트 전 화면 페이드 아웃 시간 (초). 0이면 즉시"))
    float FadeOutDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallReset", meta=(ToolTip="텔레포트 후 화면 페이드 인 시간 (초). 0이면 즉시"))
    float FadeInDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallReset", meta=(ToolTip="텔레포트 시 속도 초기화 여부"))
    bool bResetVelocity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallReset", meta=(ClampMin="0", ToolTip="떨어질 때 받는 피해. 0이면 피해 없이 돌아오기만 한다"))
    float FallDamage = 0.f;

private:
    bool bIsTeleporting = false;
    FTimerHandle TeleportTimerHandle;

    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    void DoTeleport(ACharacter* Player, APlayerController* PC);
};
