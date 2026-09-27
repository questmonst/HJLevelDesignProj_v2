// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelInteractionData.generated.h"

class USoundBase;

// 스위치·문 상태 조명 색 (DA_InteractColor). 여러 액터가 같은 에셋을 공유해 색을 한 번에 바꾼다
UCLASS(BlueprintType)
class GAMEANIMATIONSAMPLE2_API UInteractColorData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color", meta=(ToolTip="사용 불가(비활성화) 색 — 적색"))
	FLinearColor DisabledColor = FLinearColor(1.f, 0.08f, 0.05f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color", meta=(ToolTip="사용 가능(활성화) 색 — 청색"))
	FLinearColor EnabledColor = FLinearColor(0.1f, 0.45f, 1.f);
};

// 스위치 소리 (DA_InteractSound)
UCLASS(BlueprintType)
class GAMEANIMATIONSAMPLE2_API UInteractSoundData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta=(ToolTip="E로 상호작용했을 때 소리"))
	USoundBase* InteractSound = nullptr;
};

// 문 소리 (DA_InteractSoundDoor)
UCLASS(BlueprintType)
class GAMEANIMATIONSAMPLE2_API UInteractDoorSoundData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta=(ToolTip="E로 상호작용했을 때 소리 (수동문)"))
	USoundBase* InteractSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta=(ToolTip="문이 열릴 때 소리"))
	USoundBase* OpenSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta=(ToolTip="문이 닫힐 때 소리"))
	USoundBase* CloseSound = nullptr;
};
