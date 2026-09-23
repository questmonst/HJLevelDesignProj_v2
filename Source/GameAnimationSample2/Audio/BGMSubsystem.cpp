// Copyright Epic Games, Inc. All Rights Reserved.

#include "BGMSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UBGMSubsystem::SetBGMData(UBGMDataAsset* InData)
{
	BGMData = InData;
}

void UBGMSubsystem::StartBGM()
{
	PlayTrack(GetTrackForState(CurrentState));
}

void UBGMSubsystem::StopBGM()
{
	if (ActiveMusic)
	{
		const float FadeOut = BGMData ? BGMData->FadeOutTime : 1.f;
		ActiveMusic->FadeOut(FadeOut, 0.f);
		ActiveMusic = nullptr;
	}
}

void UBGMSubsystem::SetState(EBGMState NewState)
{
	if (NewState == CurrentState) return;

	CurrentState = NewState;
	PlayTrack(GetTrackForState(CurrentState));
}

void UBGMSubsystem::NotifyCombat()
{
	// 보스전 중에는 전투 BGM으로 내리지 않는다
	if (CurrentState != EBGMState::Boss)
	{
		SetState(EBGMState::Combat);
	}

	// 신호가 올 때마다 복귀 타이머를 미룬다 — 마지막 신호 후 조용해지면 탐색으로
	const float Delay = BGMData ? BGMData->CombatExitDelay : 6.f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CombatExitTimerHandle, this,
			&UBGMSubsystem::ReturnToExplore, FMath::Max(Delay, 0.1f), false);
	}
}

void UBGMSubsystem::ReturnToExplore()
{
	if (CurrentState == EBGMState::Combat)
	{
		SetState(EBGMState::Explore);
	}
}

USoundBase* UBGMSubsystem::GetTrackForState(EBGMState State) const
{
	if (!BGMData) return nullptr;

	switch (State)
	{
	case EBGMState::Combat: return BGMData->CombatBGM;
	case EBGMState::Boss:   return BGMData->BossBGM;
	default:                return BGMData->ExploreBGM;
	}
}

void UBGMSubsystem::PlayTrack(USoundBase* Track)
{
	const float FadeOut = BGMData ? BGMData->FadeOutTime : 1.f;
	const float FadeIn  = BGMData ? BGMData->FadeInTime  : 1.f;
	const float Volume  = BGMData ? BGMData->Volume      : 1.f;

	if (ActiveMusic)
	{
		ActiveMusic->FadeOut(FadeOut, 0.f);   // 이전 곡은 스스로 정리된다
		ActiveMusic = nullptr;
	}

	if (!Track) return;

	ActiveMusic = UGameplayStatics::SpawnSound2D(this, Track, Volume, 1.f, 0.f, nullptr, /*bPersistAcrossLevelTransition*/ false, /*bAutoDestroy*/ false);
	if (ActiveMusic)
	{
		ActiveMusic->FadeIn(FadeIn, Volume);
	}
}

void UBGMSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CombatExitTimerHandle);
	}
	if (ActiveMusic)
	{
		ActiveMusic->Stop();
		ActiveMusic = nullptr;
	}
	Super::Deinitialize();
}
