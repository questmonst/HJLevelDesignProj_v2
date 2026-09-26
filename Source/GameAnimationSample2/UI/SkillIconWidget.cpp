// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkillIconWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MikaCharacter.h"
#include "Kismet/GameplayStatics.h"

TSharedRef<SWidget> USkillIconWidget::RebuildWidget()
{
	if (!IconBox && WidgetTree)
	{
		BuildLayout();
	}
	return Super::RebuildWidget();
}

void USkillIconWidget::BuildLayout()
{
	IconBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("IconBox"));
	IconBox->SetWidthOverride(IconSize.X);
	IconBox->SetHeightOverride(IconSize.Y);
	WidgetTree->RootWidget = IconBox;

	IconLayers = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("IconLayers"));
	IconBox->AddChild(IconLayers);

	// 1) 아이콘 본체
	IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("IconImage"));
	if (IconTexture) IconImage->SetBrushFromTexture(IconTexture, false);
	IconImage->SetColorAndOpacity(ReadyTint);
	if (UOverlaySlot* IconSlot = IconLayers->AddChildToOverlay(IconImage))
	{
		IconSlot->SetHorizontalAlignment(HAlign_Fill);
		IconSlot->SetVerticalAlignment(VAlign_Fill);
	}

	// 2) 시계방향 덮개 — 머티리얼이 Percent 파라미터로 각도를 그린다
	SweepImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("SweepImage"));
	if (SweepMaterial)
	{
		SweepMID = UMaterialInstanceDynamic::Create(SweepMaterial, this);
		SweepImage->SetBrushFromMaterial(SweepMID);
	}
	SweepImage->SetVisibility(ESlateVisibility::Hidden);
	if (UOverlaySlot* SweepSlot = IconLayers->AddChildToOverlay(SweepImage))
	{
		SweepSlot->SetHorizontalAlignment(HAlign_Fill);
		SweepSlot->SetVerticalAlignment(VAlign_Fill);
	}

	// 3) 준비 완료 번쩍임
	FlashImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("FlashImage"));
	FlashImage->SetColorAndOpacity(FLinearColor(FlashColor.R, FlashColor.G, FlashColor.B, 0.f));
	FlashImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (UOverlaySlot* FlashSlot = IconLayers->AddChildToOverlay(FlashImage))
	{
		FlashSlot->SetHorizontalAlignment(HAlign_Fill);
		FlashSlot->SetVerticalAlignment(VAlign_Fill);
	}

	// 4) 개수·키 안내
	CountText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CountText"));
	CountText->SetText(KeyLabel);
	CountText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	CountText->SetVisibility(KeyLabel.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	if (UOverlaySlot* TextSlot = IconLayers->AddChildToOverlay(CountText))
	{
		TextSlot->SetHorizontalAlignment(HAlign_Right);
		TextSlot->SetVerticalAlignment(VAlign_Bottom);
	}
}

void USkillIconWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!IconBox) BuildLayout();

	// 위젯 트리는 클래스에서 복제돼 오므로, 디자이너에서 바꾼 값들을 여기서 다시 적용한다.
	// 특히 머티리얼 인스턴스는 컴파일 시점에 만들어진 것이라 런타임에 새로 만들어야 한다.
	RefreshVisualsFromSettings();

	Mika = Cast<AMikaCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	bWasReady = true;
	ApplyCooldownVisual(0.f, 1.f);
}

void USkillIconWidget::RefreshVisualsFromSettings()
{
	if (IconBox)
	{
		IconBox->SetWidthOverride(IconSize.X);
		IconBox->SetHeightOverride(IconSize.Y);
	}
	if (IconImage && IconTexture)
	{
		IconImage->SetBrushFromTexture(IconTexture, false);
	}
	if (SweepImage)
	{
		if (SweepMaterial)
		{
			SweepMID = UMaterialInstanceDynamic::Create(SweepMaterial, this);
			SweepMID->SetVectorParameterValue(TEXT("SweepColor"), SweepColor);
			SweepMID->SetScalarParameterValue(TEXT("Percent"), 1.f);
			SweepImage->SetBrushFromMaterial(SweepMID);
		}
		else
		{
			SweepMID = nullptr;
		}
	}
	if (CountText)
	{
		CountText->SetText(KeyLabel);
	}
}

void USkillIconWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);

	if (SkillType != ESkillSlot::Custom) ReadFromPlayer();

	// 번쩍임 감쇠
	if (FlashTimeLeft > 0.f && FlashImage)
	{
		FlashTimeLeft = FMath::Max(FlashTimeLeft - DeltaTime, 0.f);
		const float Ratio = (FlashDuration > 0.f) ? (FlashTimeLeft / FlashDuration) : 0.f;
		FlashImage->SetColorAndOpacity(FLinearColor(FlashColor.R, FlashColor.G, FlashColor.B, FlashColor.A * Ratio));
	}
}

void USkillIconWidget::ReadFromPlayer()
{
	if (!Mika.IsValid())
	{
		Mika = Cast<AMikaCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (!Mika.IsValid()) return;
	}

	switch (SkillType)
	{
	case ESkillSlot::Punch:
		ApplyCooldownVisual(Mika->GetPunchCooldownRemaining(), Mika->GetPunchCooldownDuration());
		break;

	case ESkillSlot::Grenade:
		// 수류탄은 쿨타임이 거의 없다 — 남은 개수만 보여주고, 다 떨어지면 어둡게
		SetCount(Mika->GetGrenadeCount());
		ApplyCooldownVisual(Mika->GetGrenadeCount() > 0 ? 0.f : 1.f, 1.f);
		break;

	default:
		break;
	}
}

void USkillIconWidget::SetCooldown(float Remaining, float Duration)
{
	ApplyCooldownVisual(Remaining, Duration);
}

void USkillIconWidget::ApplyCooldownVisual(float Remaining, float Duration)
{
	const bool bReady = Remaining <= KINDA_SMALL_NUMBER;

	// 쿨타임이 막 끝난 순간 한 번 번쩍인다
	if (bReady && !bWasReady)
	{
		FlashTimeLeft = FlashDuration;
	}
	bWasReady = bReady;

	if (IconImage) IconImage->SetColorAndOpacity(bReady ? ReadyTint : CooldownTint);

	if (SweepImage)
	{
		SweepImage->SetVisibility(bReady ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
		if (!bReady && SweepMID)
		{
			// 남은 시간 비율 — 1에서 시작해 0으로 줄며 덮개가 걷힌다
			const float Percent = FMath::Clamp(Remaining / FMath::Max(Duration, KINDA_SMALL_NUMBER), 0.f, 1.f);
			SweepMID->SetScalarParameterValue(TEXT("Percent"), Percent);
			SweepMID->SetVectorParameterValue(TEXT("SweepColor"), SweepColor);
		}
	}
}

void USkillIconWidget::SetCount(int32 Count)
{
	if (!CountText) return;

	if (Count < 0)
	{
		CountText->SetVisibility(KeyLabel.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		CountText->SetText(KeyLabel);
		return;
	}
	CountText->SetVisibility(ESlateVisibility::HitTestInvisible);
	CountText->SetText(FText::AsNumber(Count));
}
