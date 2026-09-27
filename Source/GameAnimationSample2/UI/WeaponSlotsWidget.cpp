// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponSlotsWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "PlayerCharacter.h"
#include "WeaponBase.h"

TSharedRef<SWidget> UWeaponSlotsWidget::RebuildWidget()
{
	if (!SlotRoot && WidgetTree)
	{
		BuildLayout();
	}
	return Super::RebuildWidget();
}

void UWeaponSlotsWidget::BuildLayout()
{
	SlotRoot = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SlotRoot"));
	WidgetTree->RootWidget = SlotRoot;

	SlotTexts.Reset();
	const int32 Count = FMath::Clamp(SlotCount, 1, 8);
	for (int32 i = 0; i < Count; ++i)
	{
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (UHorizontalBoxSlot* TextSlot = SlotRoot->AddChildToHorizontalBox(Text))
		{
			TextSlot->SetVerticalAlignment(VAlign_Center);
			TextSlot->SetPadding(FMargin(i > 0 ? SlotSpacing : 0.f, 0.f, 0.f, 0.f));
		}
		SlotTexts.Add(Text);
	}
}

void UWeaponSlotsWidget::ApplyStyle()
{
	for (int32 i = 0; i < SlotTexts.Num(); ++i)
	{
		UTextBlock* Text = SlotTexts[i];
		if (!Text) continue;

		// 글꼴 에셋을 안 골랐으면 기본 글꼴에 크기만 적용
		if (Font.HasValidFont())
		{
			Text->SetFont(Font);
		}
		else
		{
			FSlateFontInfo Default = Text->GetFont();
			Default.Size = Font.Size;
			Text->SetFont(Default);
		}

		if (UHorizontalBoxSlot* TextSlot = Cast<UHorizontalBoxSlot>(Text->Slot))
		{
			TextSlot->SetPadding(FMargin(i > 0 ? SlotSpacing : 0.f, 0.f, 0.f, 0.f));
		}
	}
}

void UWeaponSlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!SlotRoot) BuildLayout();

	// 위젯 트리는 클래스에서 복제돼 오므로 디자이너 설정값(글꼴·간격)을 여기서 다시 적용
	ApplyStyle();
	Refresh();
}

void UWeaponSlotsWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);
	Refresh();
}

void UWeaponSlotsWidget::Refresh()
{
	const APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	const TArray<AWeaponBase*>* Inventory = Player ? &Player->GetWeaponInventory() : nullptr;
	const AWeaponBase* Current = Player ? Player->GetCurrentWeapon() : nullptr;
	const int32 UsableSlots = Player ? Player->GetMaxWeaponSlots() : SlotTexts.Num();

	for (int32 i = 0; i < SlotTexts.Num(); ++i)
	{
		UTextBlock* Text = SlotTexts[i];
		if (!Text) continue;

		// 플레이어가 들 수 있는 수보다 많은 칸은 숨긴다 (MikaData › MaxWeaponSlots)
		if (i >= UsableSlots)
		{
			Text->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		const AWeaponBase* Weapon = (Inventory && Inventory->IsValidIndex(i)) ? (*Inventory)[i] : nullptr;
		if (Weapon)
		{
			Text->SetText(FText::Format(SlotFormat, Weapon->GetWeaponDisplayName()));
			Text->SetColorAndOpacity(Weapon == Current ? EquippedColor : InventoryColor);
			Text->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else if (!EmptyText.IsEmpty())
		{
			Text->SetText(EmptyText);
			Text->SetColorAndOpacity(EmptyColor);
			Text->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Text->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
