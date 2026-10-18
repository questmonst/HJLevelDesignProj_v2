// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeathScreenWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "PlayerCharacter.h"

UDeathScreenWidget::UDeathScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TitleFont.Size  = 48;
	ButtonFont.Size = 22;
	SetIsFocusable(true);
}

TSharedRef<SWidget> UDeathScreenWidget::RebuildWidget()
{
	if (!Title && WidgetTree)
	{
		BuildLayout();
	}
	return Super::RebuildWidget();
}

void UDeathScreenWidget::BuildLayout()
{
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("DeathRoot"));
	WidgetTree->RootWidget = Root;

	// 화면 전체를 어둡게
	Dim = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Dim"));
	if (UOverlaySlot* DimSlot = Root->AddChildToOverlay(Dim))
	{
		DimSlot->SetHorizontalAlignment(HAlign_Fill);
		DimSlot->SetVerticalAlignment(VAlign_Fill);
	}

	// 가운데: 제목 + 버튼
	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
	if (UOverlaySlot* ColumnSlot = Root->AddChildToOverlay(Column))
	{
		ColumnSlot->SetHorizontalAlignment(HAlign_Center);
		ColumnSlot->SetVerticalAlignment(VAlign_Center);
	}

	Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
	Title->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
		TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 40.f));
	}

	RestartButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RestartButton"));
	if (UVerticalBoxSlot* ButtonSlot = Column->AddChildToVerticalBox(RestartButton))
	{
		ButtonSlot->SetHorizontalAlignment(HAlign_Center);
	}

	RestartLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestartLabel"));
	RestartLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f)));
	RestartButton->AddChild(RestartLabel);
}

static void ApplyFont(UTextBlock* Text, const FSlateFontInfo& Font)
{
	if (!Text) return;
	if (Font.HasValidFont())
	{
		Text->SetFont(Font);
		return;
	}
	FSlateFontInfo Default = Text->GetFont();
	Default.Size = Font.Size;
	Text->SetFont(Default);
}

void UDeathScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!Title) BuildLayout();

	// 위젯 트리는 클래스에서 복제돼 오므로 디자이너 설정값을 여기서 다시 적용
	Dim->SetColorAndOpacity(DimColor);
	Title->SetText(TitleText);
	Title->SetColorAndOpacity(TitleColor);
	ApplyFont(Title, TitleFont);
	RestartLabel->SetText(RestartText);
	ApplyFont(RestartLabel, ButtonFont);

	RestartButton->OnClicked.AddUniqueDynamic(this, &UDeathScreenWidget::HandleRestartClicked);
	RestartButton->SetKeyboardFocus();   // Enter·Space·게임패드 A로도 누를 수 있게
}

void UDeathScreenWidget::HandleRestartClicked()
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
	{
		Player->RespawnAtCheckpoint();   // 이 위젯을 닫는 것도 플레이어가 한다
	}
}
