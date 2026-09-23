// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerHealthBarWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "CharacterBase.h"
#include "Kismet/GameplayStatics.h"

TSharedRef<SWidget> UPlayerHealthBarWidget::RebuildWidget()
{
	// 디자이너에 아무것도 없어도 칸이 보이도록, 위젯 트리를 코드가 직접 만든다
	if (!SegmentRow && WidgetTree)
	{
		SegmentRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SegmentRow"));
		WidgetTree->RootWidget = SegmentRow;
		BuildSegments();
	}
	return Super::RebuildWidget();
}

void UPlayerHealthBarWidget::BuildSegments()
{
	if (!SegmentRow || !WidgetTree) return;

	SegmentRow->ClearChildren();
	Segments.Reset();

	const int32 Count = FMath::Max(SegmentCount, 1);
	for (int32 i = 0; i < Count; ++i)
	{
		// 칸 하나 = 고정 크기 상자 안의 작은 프로그레스바
		USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Box->SetWidthOverride(SegmentSize.X);
		Box->SetHeightOverride(SegmentSize.Y);

		UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());

		FProgressBarStyle Style;
		FSlateBrush Background;
		Background.DrawAs = ESlateBrushDrawType::RoundedBox;
		Background.OutlineSettings = FSlateBrushOutlineSettings(
			FVector4(CornerRadius, CornerRadius, CornerRadius, CornerRadius), OutlineColor, OutlineWidth);
		Background.TintColor = FSlateColor(EmptyColor);

		FSlateBrush Fill = Background;
		Fill.OutlineSettings.Color = FSlateColor(FLinearColor::Transparent);
		Fill.TintColor = FSlateColor(FillColor);

		Style.SetBackgroundImage(Background);
		Style.SetFillImage(Fill);
		Bar->SetWidgetStyle(Style);
		Bar->SetFillColorAndOpacity(FLinearColor::White);   // 브러시 색을 그대로 쓴다
		Bar->SetBarFillStyle(EProgressBarFillStyle::Scale);
		Bar->SetPercent(1.f);

		Box->AddChild(Bar);
		if (UHorizontalBoxSlot* BoxSlot = SegmentRow->AddChildToHorizontalBox(Box))
		{
			// 마지막 칸 뒤에는 간격을 두지 않는다
			BoxSlot->SetPadding(FMargin(0.f, 0.f, (i == Count - 1) ? 0.f : SegmentGap, 0.f));
		}
		Segments.Add(Bar);
	}
}

void UPlayerHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (Segments.Num() == 0) BuildSegments();
	if (bAutoBindToPlayer) BindToPlayer();
	RefreshSegments();
}

void UPlayerHealthBarWidget::NativeDestruct()
{
	if (BoundCharacter.IsValid())
	{
		BoundCharacter->OnHealthChanged.RemoveDynamic(this, &UPlayerHealthBarWidget::HandleHealthChanged);
	}
	Super::NativeDestruct();
}

void UPlayerHealthBarWidget::BindToPlayer()
{
	ACharacterBase* Player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player) return;

	BoundCharacter = Player;
	Player->OnHealthChanged.AddUniqueDynamic(this, &UPlayerHealthBarWidget::HandleHealthChanged);

	CachedCurrent = Player->GetCurrentHealth();
	CachedMax     = Player->GetMaxHealth();
}

void UPlayerHealthBarWidget::HandleHealthChanged(float Current, float Max)
{
	CachedCurrent = Current;
	CachedMax     = Max;
	RefreshSegments();
}

void UPlayerHealthBarWidget::SetHealth(float Current, float Max)
{
	CachedCurrent = Current;
	CachedMax     = Max;
	RefreshSegments();
}

void UPlayerHealthBarWidget::RefreshSegments()
{
	const int32 Count = Segments.Num();
	if (Count == 0) return;

	// 칸 하나가 담는 체력. 최대 체력이 바뀌어도 칸 수는 그대로다
	const float PerSegment = FMath::Max(CachedMax, KINDA_SMALL_NUMBER) / Count;

	for (int32 i = 0; i < Count; ++i)
	{
		if (!Segments[i]) continue;
		// i번째 칸은 (i * PerSegment) ~ ((i+1) * PerSegment) 구간을 담당한다
		const float Filled = (CachedCurrent - PerSegment * i) / PerSegment;
		Segments[i]->SetPercent(FMath::Clamp(Filled, 0.f, 1.f));
	}
}
