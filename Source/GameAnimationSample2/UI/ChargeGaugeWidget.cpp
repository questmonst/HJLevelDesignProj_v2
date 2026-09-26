// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChargeGaugeWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MikaCharacter.h"
#include "Kismet/GameplayStatics.h"

TSharedRef<SWidget> UChargeGaugeWidget::RebuildWidget()
{
	if (!GaugeBox && WidgetTree)
	{
		BuildLayout();
	}
	return Super::RebuildWidget();
}

void UChargeGaugeWidget::BuildLayout()
{
	GaugeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("GaugeBox"));
	GaugeBox->SetWidthOverride(GaugeSize.X);
	GaugeBox->SetHeightOverride(GaugeSize.Y);
	WidgetTree->RootWidget = GaugeBox;

	ArcImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ArcImage"));
	ArcImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	GaugeBox->AddChild(ArcImage);
}

void UChargeGaugeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!GaugeBox) BuildLayout();

	// 위젯 트리는 클래스에서 복제돼 오므로 머티리얼 인스턴스를 런타임에 새로 만든다
	RefreshVisualsFromSettings();

	Mika = Cast<AMikaCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));

	// 루트(this)를 숨기면 안 된다. Slate는 Tick을 Paint 안에서 부르기 때문에
	// 안 그려지는 위젯은 틱도 돌지 않아 스스로 다시 나타날 수 없다. 안쪽 이미지만 숨긴다
	if (ArcImage) ArcImage->SetVisibility(ESlateVisibility::Hidden);
}

void UChargeGaugeWidget::RefreshVisualsFromSettings()
{
	if (GaugeBox)
	{
		GaugeBox->SetWidthOverride(GaugeSize.X);
		GaugeBox->SetHeightOverride(GaugeSize.Y);
	}
	if (!ArcImage) return;

	if (ArcMaterial)
	{
		ArcMID = UMaterialInstanceDynamic::Create(ArcMaterial, this);
		ArcMID->SetVectorParameterValue(TEXT("BgColor"), BackgroundColor);
		ArcMID->SetVectorParameterValue(TEXT("FillColor"), FillColor);
		ArcMID->SetVectorParameterValue(TEXT("MarkerColor"), MarkerColor);
		ArcMID->SetScalarParameterValue(TEXT("Percent"), 0.f);
		ArcMID->SetScalarParameterValue(TEXT("Marker"), 0.75f);
		ArcImage->SetBrushFromMaterial(ArcMID);
	}
	else
	{
		ArcMID = nullptr;
	}
}

void UChargeGaugeWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);

	if (!Mika.IsValid())
	{
		Mika = Cast<AMikaCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (!Mika.IsValid()) return;
	}

	if (!ArcImage) return;

	if (!Mika->IsPunchCharging())
	{
		if (ArcImage->GetVisibility() != ESlateVisibility::Hidden)
		{
			ArcImage->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	if (ArcImage->GetVisibility() != ESlateVisibility::HitTestInvisible)
	{
		ArcImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (!ArcMID) return;

	const float Percent = Mika->GetPunchChargeForcedRatio();
	const float Marker  = Mika->GetPunchChargeMarkerRatio();

	ArcMID->SetScalarParameterValue(TEXT("Percent"), Percent);
	ArcMID->SetScalarParameterValue(TEXT("Marker"), Marker);
	// 최대 위력에 닿으면 채운 부분이 전부 흰색이 된다
	ArcMID->SetVectorParameterValue(TEXT("FillColor"), (Percent >= Marker) ? FullColor : FillColor);
}
