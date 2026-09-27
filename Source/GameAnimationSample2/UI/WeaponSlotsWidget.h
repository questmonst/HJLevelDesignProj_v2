// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "WeaponSlotsWidget.generated.h"

class UHorizontalBox;
class UTextBlock;

/**
 * 무기 슬롯 표시 — (무기 1) (무기 2) (무기 3)
 *
 * 인벤토리 순서대로 무기 이름을 보여준다. 들고 있는 무기는 흰색, 가방에 있는 무기는 회색.
 * 빈 칸은 EmptyText (비우면 칸 자체를 숨김). 매 틱 플레이어 인벤토리를 읽어 갱신한다.
 *
 * 칸은 코드가 만들기 때문에 WBP 디자이너는 비워둬도 된다.
 * HUD(WBP_Crosshair_V2 등)에 얹을 때는 사람이 직접 계층구조에 드래그 앤 드롭하고, 앵커를 우하단으로.
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API UWeaponSlotsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlots", meta=(ClampMin="1", ClampMax="8", ToolTip="표시할 칸 수 (플레이어 최대 무기 슬롯과 맞출 것)"))
	int32 SlotCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlots", meta=(ToolTip="칸 하나의 표시 형식. {0} = 무기 이름"))
	FText SlotFormat = FText::FromString(TEXT("{0}"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlots", meta=(ToolTip="빈 칸에 표시할 글자. 비우면 빈 칸은 숨긴다"))
	FText EmptyText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlots", meta=(ToolTip="들고 있는 무기 색"))
	FSlateColor EquippedColor = FSlateColor(FLinearColor::White);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlots", meta=(ToolTip="가방에 있는 무기 색"))
	FSlateColor InventoryColor = FSlateColor(FLinearColor(0.45f, 0.45f, 0.45f, 1.f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlots", meta=(ToolTip="빈 칸 색"))
	FSlateColor EmptyColor = FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f, 0.6f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlots", meta=(ToolTip="글꼴. 비우면(Font Family 없음) 텍스트 기본 글꼴을 크기만 바꿔 쓴다"))
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlots", meta=(ClampMin="0", ToolTip="칸 사이 간격(px)"))
	float SlotSpacing = 24.f;

	// --- 코드가 만드는 위젯 ---

	UPROPERTY(Transient) UHorizontalBox* SlotRoot = nullptr;
	UPROPERTY(Transient) TArray<UTextBlock*> SlotTexts;

	void BuildLayout();
	void ApplyStyle();
	void Refresh();
};
