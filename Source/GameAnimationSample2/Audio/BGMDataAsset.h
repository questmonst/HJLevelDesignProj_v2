// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BGMDataAsset.generated.h"

class USoundBase;

// 전투 상황에 따른 BGM 단계. 위로 갈수록 우선순위가 높다.
UENUM(BlueprintType)
enum class EBGMState : uint8
{
	Explore  UMETA(DisplayName = "탐색"),
	Combat   UMETA(DisplayName = "전투"),
	Boss     UMETA(DisplayName = "보스")
};

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UBGMDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Tracks ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tracks", meta=(ToolTip="평상시(탐색) BGM. 루프되는 사운드를 넣는다"))
	USoundBase* ExploreBGM = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tracks", meta=(ToolTip="전투 BGM"))
	USoundBase* CombatBGM = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tracks", meta=(ToolTip="보스전 BGM"))
	USoundBase* BossBGM = nullptr;

	// --- Blend ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend", meta=(ClampMin="0", ToolTip="이전 곡이 사라지는 시간 (초)"))
	float FadeOutTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend", meta=(ClampMin="0", ToolTip="새 곡이 올라오는 시간 (초)"))
	float FadeInTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend", meta=(ClampMin="0", ToolTip="BGM 볼륨 배율"))
	float Volume = 1.f;

	// --- Combat ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta=(ClampMin="0", ToolTip="마지막 전투 신호(피격·발사·적 발견) 후 이 시간이 지나면 탐색 BGM으로 돌아간다 (초)"))
	float CombatExitDelay = 6.f;
};
