// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallResetTrigger.generated.h"

class UBoxComponent;
class AFallResetTarget;

/**
 * 낙사 리셋 트리거 (Actor A).
 * 이 액터의 콜리전에 플레이어가 닿으면 연결된 AFallResetTarget 위치로 순간이동.
 *
 * 사용법:
 *   1. 레벨에 FallResetTrigger 배치 (낙사 구역 아래 등)
 *   2. Detail 패널에서 ResetTarget에 FallResetTarget 액터 연결
 *   3. 필요시 FadeOutDuration, FadeInDuration 조정
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API AFallResetTrigger : public AActor
{
    GENERATED_BODY()

public:
    AFallResetTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FallReset")
    UBoxComponent* TriggerBox;

    /** 이동할 목표 위치 액터 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallReset")
    AFallResetTarget* ResetTarget;

    /** 텔레포트 전 화면 페이드 아웃 시간 (0 = 즉시) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallReset")
    float FadeOutDuration = 0.2f;

    /** 텔레포트 후 화면 페이드 인 시간 (0 = 즉시) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallReset")
    float FadeInDuration = 0.3f;

    /** 텔레포트 시 속도 초기화 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallReset")
    bool bResetVelocity = true;

private:
    bool bIsTeleporting = false;
    FTimerHandle TeleportTimerHandle;

    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    void DoTeleport(ACharacter* Player, APlayerController* PC);
};
