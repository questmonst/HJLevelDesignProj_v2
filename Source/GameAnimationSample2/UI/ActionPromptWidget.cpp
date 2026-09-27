// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionPromptWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "PlayerCharacter.h"

TSharedRef<SWidget> UActionPromptWidget::RebuildWidget()
{
	if (!PromptText && WidgetTree)
	{
		BuildLayout();
	}
	return Super::RebuildWidget();
}

void UActionPromptWidget::BuildLayout()
{
	// 루트는 숨기지 않고 안쪽 텍스트만 숨긴다 — 루트를 숨기면 틱이 멈춰 다시 나타날 수 없다
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("PromptRoot"));
	WidgetTree->RootWidget = Root;

	PromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PromptText"));
	PromptText->SetJustification(ETextJustify::Center);
	if (UOverlaySlot* TextSlot = Root->AddChildToOverlay(PromptText))
	{
		TextSlot->SetHorizontalAlignment(HAlign_Center);
		TextSlot->SetVerticalAlignment(VAlign_Center);
	}
}

void UActionPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!PromptText) BuildLayout();

	// 위젯 트리는 클래스에서 복제돼 오므로 디자이너 설정값을 여기서 다시 적용
	if (Font.HasValidFont())
	{
		PromptText->SetFont(Font);
	}
	else
	{
		FSlateFontInfo Default = PromptText->GetFont();
		Default.Size = Font.Size;
		PromptText->SetFont(Default);
	}
	PromptText->SetColorAndOpacity(TextColor);
	PromptText->SetVisibility(ESlateVisibility::Hidden);
}

void UActionPromptWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);
	if (!PromptText) return;

	const APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	const FText Text = Player ? Player->GetActionPromptText() : FText::GetEmpty();

	if (Text.IsEmpty())
	{
		if (PromptText->GetVisibility() != ESlateVisibility::Hidden) PromptText->SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	if (!PromptText->GetText().EqualTo(Text)) PromptText->SetText(Text);
	if (PromptText->GetVisibility() != ESlateVisibility::HitTestInvisible) PromptText->SetVisibility(ESlateVisibility::HitTestInvisible);
}
