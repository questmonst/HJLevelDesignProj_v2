// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

class UWidgetAnimation;

/**
 * 크로스헤어 HUD.
 *
 * 이미 WBP에 만들어 둔 히트마커 애니메이션(Anim_Hit / Anim_HeadShot / Anim_Killed)을
 * C++이 재생해 준다. 플레이어가 적을 맞히면 `APlayerCharacter::OnHitConfirmed`가 방송되고,
 * 이 위젯이 그걸 받아서 알맞은 애니를 튼다. WBP 그래프에 배선할 필요 없음.
 *
 * WBP 쪽 준비물: 같은 이름의 위젯 애니메이션 3개 (이름이 다르면 아래 변수명을 맞출 것)
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 일반 적중 마커 (흰 X)
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void PlayHitMarker();

	// 헤드샷 마커 (빨간 X) — 없으면 일반 마커로 대체
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void PlayHeadShotMarker();

	// 처치 마커
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void PlayKillMarker();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleHitConfirmed(bool bHeadshot);

	UFUNCTION()
	void HandleEnemyKilled();

	// WBP의 애니메이션과 이름으로 연결된다 (없으면 nullptr — 그냥 재생만 건너뜀)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Crosshair", meta=(BindWidgetAnimOptional))
	UWidgetAnimation* Anim_Hit = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Crosshair", meta=(BindWidgetAnimOptional))
	UWidgetAnimation* Anim_HeadShot = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Crosshair", meta=(BindWidgetAnimOptional))
	UWidgetAnimation* Anim_Killed = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair", meta=(ToolTip="true면 플레이어의 적중·처치 이벤트에 자동으로 연결된다"))
	bool bAutoBindToPlayer = true;
};
