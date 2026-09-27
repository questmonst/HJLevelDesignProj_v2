// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "ActionPromptWidget.generated.h"

class UTextBlock;

/**
 * 행동 안내 문구 — "Q키를 눌러 왼쪽으로 기울이기" 같은 한 줄.
 *
 * 매 틱 APlayerCharacter::GetActionPromptText()를 읽어, 문구가 있으면 보이고 없으면 숨긴다.
 * 문구 내용은 HUDData › Prompt에서 바꾼다.
 *
 * 텍스트는 코드가 만들기 때문에 WBP 디자이너는 비워둬도 된다.
 * HUD에 얹을 때는 사람이 직접 계층구조에 드래그 앤 드롭할 것.
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API UActionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionPrompt", meta=(ToolTip="글꼴. 비우면(Font Family 없음) 텍스트 기본 글꼴을 크기만 바꿔 쓴다"))
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionPrompt", meta=(ToolTip="글자 색"))
	FSlateColor TextColor = FSlateColor(FLinearColor::White);

	UPROPERTY(Transient) UTextBlock* PromptText = nullptr;

	void BuildLayout();
};
