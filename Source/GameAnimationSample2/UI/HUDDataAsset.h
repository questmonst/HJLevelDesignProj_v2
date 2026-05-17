// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HUDDataAsset.generated.h"

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UHUDDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Crosshair Shape ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Shape", meta=(ToolTip="크로스헤어 선의 길이 (픽셀)"))
	float CrosshairSize = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Shape", meta=(ToolTip="크로스헤어 선의 두께 (픽셀)"))
	float CrosshairThickness = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Shape", meta=(ToolTip="중앙에서 크로스헤어 선까지의 간격 (픽셀). 클수록 중앙이 비어 보임"))
	float CrosshairGap = 8.f;

	// --- Crosshair Color ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Color", meta=(ToolTip="기본 크로스헤어 색상"))
	FLinearColor CrosshairColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Color", meta=(ToolTip="조준(ADS) 시 크로스헤어 색상"))
	FLinearColor CrosshairAimColor = FLinearColor(1.f, 0.8f, 0.1f, 1.f);

	// --- Crosshair Spread ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Spread", meta=(ToolTip="걷는 동안 추가되는 크로스헤어 퍼짐량"))
	float SpreadWalking = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Spread", meta=(ToolTip="달리는 동안 추가되는 크로스헤어 퍼짐량"))
	float SpreadSprinting = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Spread", meta=(ToolTip="조준 시 퍼짐 변화량 (음수=좁아짐). 기본값 -5"))
	float SpreadAimingDelta = -5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Spread", meta=(ToolTip="초당 크로스헤어 퍼짐 회복량"))
	float SpreadRecoverySpeed = 15.f;
};
