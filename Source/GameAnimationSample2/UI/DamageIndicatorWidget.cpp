// Copyright Epic Games, Inc. All Rights Reserved.

#include "DamageIndicatorWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Engine/Texture2D.h"
#include "CharacterBase.h"
#include "Kismet/GameplayStatics.h"

TSharedRef<SWidget> UDamageIndicatorWidget::RebuildWidget()
{
	if (!IndicatorRoot && WidgetTree)
	{
		BuildLayout();
	}
	return Super::RebuildWidget();
}

void UDamageIndicatorWidget::BuildLayout()
{
	IndicatorRoot = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("IndicatorRoot"));
	WidgetTree->RootWidget = IndicatorRoot;

	Images.Reset();
	const int32 Count = FMath::Clamp(MaxIndicators, 1, 16);
	for (int32 i = 0; i < Count; ++i)
	{
		UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		// 전부 화면 중앙에 겹쳐 두고, 매 프레임 회전·이동으로 둘레에 배치한다
		if (UOverlaySlot* ImgSlot = IndicatorRoot->AddChildToOverlay(Img))
		{
			ImgSlot->SetHorizontalAlignment(HAlign_Center);
			ImgSlot->SetVerticalAlignment(VAlign_Center);
		}
		// 루트가 아니라 개별 이미지를 숨긴다 — 루트를 숨기면 틱이 멈춰 다시 나타날 수 없다
		Img->SetVisibility(ESlateVisibility::Hidden);
		Images.Add(Img);
	}
}

void UDamageIndicatorWidget::ApplyBrushes()
{
	for (UImage* Img : Images)
	{
		if (!Img) continue;

		FSlateBrush Brush;
		if (IndicatorTexture)
		{
			Brush.SetResourceObject(IndicatorTexture);
			Brush.DrawAs = ESlateBrushDrawType::Image;
		}
		else
		{
			// 텍스처가 없으면 둥근 막대
			Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
			const float R = IndicatorSize.Y * 0.5f;
			Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(R, R, R, R), FSlateColor(FLinearColor::Transparent), 0.f);
		}
		Brush.ImageSize = IndicatorSize;
		Img->SetBrush(Brush);
		Img->SetColorAndOpacity(Color);
		// 회전 중심 = 이미지 중앙
		Img->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	}
}

void UDamageIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!IndicatorRoot) BuildLayout();

	// 위젯 트리는 클래스에서 복제돼 오므로 디자이너 설정값(텍스처·크기·색)을 여기서 다시 적용
	ApplyBrushes();
	Indicators.SetNum(Images.Num());

	if (ACharacterBase* Player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Player->OnDamagedFrom.AddUniqueDynamic(this, &UDamageIndicatorWidget::HandleDamagedFrom);
	}
}

void UDamageIndicatorWidget::NativeDestruct()
{
	if (ACharacterBase* Player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Player->OnDamagedFrom.RemoveDynamic(this, &UDamageIndicatorWidget::HandleDamagedFrom);
	}
	Super::NativeDestruct();
}

float UDamageIndicatorWidget::GetRelativeYaw(const FVector& Source) const
{
	const APlayerController* PC = GetOwningPlayer();
	if (!PC) return 0.f;

	FVector  CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector ToSource = (Source - CamLoc).GetSafeNormal2D();
	if (ToSource.IsNearlyZero()) return 0.f;
	return FMath::FindDeltaAngleDegrees(CamRot.Yaw, ToSource.Rotation().Yaw);
}

void UDamageIndicatorWidget::HandleDamagedFrom(float Amount, FVector SourceLocation)
{
	if (Indicators.Num() == 0) return;

	// 같은 방향에서 연달아 맞으면 기존 표시를 갱신 (연사에 맞을 때 표시가 겹겹이 쌓이지 않게)
	const float NewYaw = GetRelativeYaw(SourceLocation);
	int32 Target = INDEX_NONE;
	for (int32 i = 0; i < Indicators.Num(); ++i)
	{
		if (Indicators[i].TimeLeft <= 0.f) continue;
		if (FMath::Abs(FMath::FindDeltaAngleDegrees(GetRelativeYaw(Indicators[i].Source), NewYaw)) <= MergeAngle)
		{
			Target = i;
			break;
		}
	}

	// 없으면 빈 칸, 그것도 없으면 가장 오래된(남은 시간이 가장 짧은) 칸을 덮어쓴다
	if (Target == INDEX_NONE)
	{
		float Shortest = TNumericLimits<float>::Max();
		for (int32 i = 0; i < Indicators.Num(); ++i)
		{
			if (Indicators[i].TimeLeft < Shortest)
			{
				Shortest = Indicators[i].TimeLeft;
				Target = i;
			}
		}
	}

	Indicators[Target].Source   = SourceLocation;
	Indicators[Target].TimeLeft = Duration;
}

void UDamageIndicatorWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);

	for (int32 i = 0; i < Indicators.Num() && i < Images.Num(); ++i)
	{
		UImage* Img = Images[i];
		if (!Img) continue;

		FIndicator& Ind = Indicators[i];
		if (Ind.TimeLeft <= 0.f)
		{
			if (Img->GetVisibility() != ESlateVisibility::Hidden) Img->SetVisibility(ESlateVisibility::Hidden);
			continue;
		}
		Ind.TimeLeft -= DeltaTime;

		// 화면 위쪽 = 정면. 몸을 돌리면 표시도 따라 돈다 (월드 위치를 기억하므로)
		const float Yaw = GetRelativeYaw(Ind.Source);
		const float Rad = FMath::DegreesToRadians(Yaw);
		Img->SetRenderTranslation(FVector2D(FMath::Sin(Rad) * Radius, -FMath::Cos(Rad) * Radius));
		Img->SetRenderTransformAngle(Yaw);

		// 끝부분에서 흐려진다
		const float Life   = FMath::Clamp(Ind.TimeLeft / FMath::Max(Duration, KINDA_SMALL_NUMBER), 0.f, 1.f);
		const float FadeAt = FMath::Clamp(1.f - FadeStartRatio, KINDA_SMALL_NUMBER, 1.f);
		Img->SetRenderOpacity(FMath::Clamp(Life / FadeAt, 0.f, 1.f));

		if (Img->GetVisibility() != ESlateVisibility::HitTestInvisible)
		{
			Img->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}
