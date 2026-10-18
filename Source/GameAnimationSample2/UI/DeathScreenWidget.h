// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "DeathScreenWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

/**
 * 사망 화면 — 화면을 어둡게 덮고 "체크포인트에서 다시 시작" 버튼을 띄운다.
 * 버튼을 누르면 플레이어가 마지막 체크포인트(없으면 플레이어 스타트)에서 되살아난다.
 *
 * 미카가 죽으면 APlayerCharacter가 DeathScreenDelay 뒤에 직접 띄운다 (HUD에 배치할 필요 없음).
 * 칸은 코드가 만들기 때문에 WBP 디자이너는 비워둬도 된다. 문구·색·글꼴은 WBP 디테일에서.
 * 버튼에 포커스를 주므로 Enter·Space·게임패드 A로도 누를 수 있다.
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API UDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathScreen", meta=(ToolTip="제목 문구"))
	FText TitleText = FText::FromString(TEXT("쓰러졌다"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathScreen", meta=(ToolTip="다시 시작 버튼 문구"))
	FText RestartText = FText::FromString(TEXT("체크포인트에서 다시 시작"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathScreen", meta=(ToolTip="화면을 덮는 색 (알파 = 어두운 정도)"))
	FLinearColor DimColor = FLinearColor(0.f, 0.f, 0.f, 0.65f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathScreen", meta=(ToolTip="제목 글꼴. Font Family가 비어 있으면 기본 글꼴에 크기만 적용"))
	FSlateFontInfo TitleFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathScreen", meta=(ToolTip="버튼 글꼴. Font Family가 비어 있으면 기본 글꼴에 크기만 적용"))
	FSlateFontInfo ButtonFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathScreen", meta=(ToolTip="제목 색"))
	FSlateColor TitleColor = FSlateColor(FLinearColor(1.f, 0.35f, 0.35f));

	UPROPERTY(Transient) UImage*     Dim = nullptr;
	UPROPERTY(Transient) UTextBlock* Title = nullptr;
	UPROPERTY(Transient) UButton*    RestartButton = nullptr;
	UPROPERTY(Transient) UTextBlock* RestartLabel = nullptr;

	void BuildLayout();

	UFUNCTION()
	void HandleRestartClicked();

public:
	UDeathScreenWidget(const FObjectInitializer& ObjectInitializer);
};
