// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelTriggerVolume.generated.h"

class UBoxComponent;
class ALevelTriggerVolume;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggerVolumeFiredSignature, ALevelTriggerVolume*, Volume);

// 밟는 트리거 — 플레이어가 상자 안에 들어오면 SignalTargets(스포너·문·스위치)에 신호를 보낸다.
// 레벨 BP 없이 "여기 들어오면 적 등장" 같은 구간을 만들 때 쓴다. 상자는 에디터에서만 보인다.
//
// 플레이어가 죽었다 다시 시작하면: 현재 체크포인트 이후에 발동했던 트리거는 다시 밟을 수 있게 된다
// (스포너도 같은 규칙으로 되돌아가므로, 같은 자리에 오면 같은 전투가 다시 시작된다)
UCLASS(Blueprintable)
class GAMEANIMATIONSAMPLE2_API ALevelTriggerVolume : public AActor
{
	GENERATED_BODY()

public:
	ALevelTriggerVolume();

	UPROPERTY(BlueprintAssignable, Category = "Trigger")
	FOnTriggerVolumeFiredSignature OnFired;

	// 다시 밟을 수 있게 (BP·레벨 연출용)
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void Rearm() { bFired = false; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger", meta=(ToolTip="밟는 영역. 크기는 배치한 뒤 Box Extent로"))
	UBoxComponent* Box;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger", meta=(ToolTip="들어왔을 때 신호를 보낼 액터 (스포너·문·스위치). 레벨에서 스포이트로 지정"))
	TArray<TObjectPtr<AActor>> SignalTargets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger", meta=(ToolTip="보낼 신호 값. true = 켜기(스폰 시작·청색·열기), false = 끄기(적 정리·적색·닫기)"))
	bool bSignalValue = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger", meta=(ToolTip="한 번만 발동. 끄면 들어올 때마다 보낸다"))
	bool bOnce = true;

	bool  bFired    = false;
	float FiredTime = -1.f;

	UFUNCTION()
	void HandleBegin(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);

	void HandlePlayerRespawned();
};
