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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Shape")
	float CrosshairSize = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Shape")
	float CrosshairThickness = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Shape")
	float CrosshairGap = 8.f;

	// --- Crosshair Color ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Color")
	FLinearColor CrosshairColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Color")
	FLinearColor CrosshairAimColor = FLinearColor(1.f, 0.8f, 0.1f, 1.f);

	// --- Crosshair Spread ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Spread")
	float SpreadWalking = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Spread")
	float SpreadSprinting = 25.f;

	/** 조준 시 퍼짐 변화량 (음수 = 좁아짐) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Spread")
	float SpreadAimingDelta = -5.f;

	/** 발사/이동으로 추가된 스프레드가 초당 줄어드는 양 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair|Spread")
	float SpreadRecoverySpeed = 15.f;
};
