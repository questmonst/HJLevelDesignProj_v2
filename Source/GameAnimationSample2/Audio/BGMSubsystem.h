// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BGMDataAsset.h"
#include "BGMSubsystem.generated.h"

class UAudioComponent;

/**
 * 전투 상황에 따라 BGM을 바꾸는 임시 뼈대.
 * 곡·페이드 시간은 전부 BGMDataAsset에서 조정한다.
 *
 * 쓰는 법:
 *   1) BGM 데이터 에셋을 만들고 곡을 넣는다
 *   2) 레벨 BP나 게임모드에서 SetBGMData(Data) → StartBGM()
 *   3) 전투가 벌어지는 지점에서 NotifyCombat() 호출 (피격·발사·적 발견 등)
 *      보스전은 SetState(Boss)로 직접 고정하고, 끝나면 SetState(Explore)
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API UBGMSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BGM", meta=(ToolTip="사용할 BGM 데이터 에셋을 지정한다"))
	void SetBGMData(UBGMDataAsset* InData);

	UFUNCTION(BlueprintCallable, Category = "BGM", meta=(ToolTip="현재 상태의 BGM 재생을 시작한다"))
	void StartBGM();

	UFUNCTION(BlueprintCallable, Category = "BGM", meta=(ToolTip="BGM을 페이드 아웃하고 멈춘다"))
	void StopBGM();

	UFUNCTION(BlueprintCallable, Category = "BGM", meta=(ToolTip="BGM 상태를 바꾼다. 같은 상태면 아무것도 하지 않는다"))
	void SetState(EBGMState NewState);

	UFUNCTION(BlueprintCallable, Category = "BGM", meta=(ToolTip="전투가 일어났음을 알린다. 전투 BGM으로 바꾸고, CombatExitDelay 동안 신호가 없으면 탐색으로 돌아간다"))
	void NotifyCombat();

	UFUNCTION(BlueprintPure, Category = "BGM")
	EBGMState GetState() const { return CurrentState; }

protected:
	virtual void Deinitialize() override;

	UPROPERTY(Transient)
	UBGMDataAsset* BGMData = nullptr;

	UPROPERTY(Transient)
	UAudioComponent* ActiveMusic = nullptr;

	EBGMState CurrentState = EBGMState::Explore;
	FTimerHandle CombatExitTimerHandle;

	USoundBase* GetTrackForState(EBGMState State) const;
	void PlayTrack(USoundBase* Track);
	void ReturnToExplore();
};
