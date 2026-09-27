// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

class UWidgetAnimation;
class UTextBlock;

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
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

	// --- 무기 표시 ---
	// BP 그래프가 이벤트(조준·줍기·시작)에서만 갱신하면 그 사이에 값이 낡고,
	// 갱신 전에는 디자이너에 적어둔 자리표시 문구("Now/Max")가 그대로 보인다.
	// 매 프레임 현재 무기에서 읽어 채우고, 무기가 없으면 숨긴다.

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Crosshair|Weapon", meta=(BindWidgetOptional))
	UTextBlock* Ammos = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Crosshair|Weapon", meta=(BindWidgetOptional, ToolTip="현재 무기 이름을 표시할 텍스트. 같은 이름으로 위젯을 만들면 자동 연결"))
	UTextBlock* WeaponName = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair|Weapon", meta=(ToolTip="잔탄 표시 형식. {0}=탄창 안 잔탄, {1}=탄창 크기, {2}=예비 탄약. 기본은 슈팅 게임 표준인 '탄창 잔탄 / 예비 탄약'"))
	FText AmmoFormat = FText::FromString(TEXT("{0} / {2}"));

	void UpdateWeaponTexts();

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
