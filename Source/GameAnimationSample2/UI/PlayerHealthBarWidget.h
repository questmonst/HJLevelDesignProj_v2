// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHealthBarWidget.generated.h"

class UHorizontalBox;
class UProgressBar;
class ACharacterBase;

/**
 * 오버워치 2 스타일 플레이어 체력바.
 *
 * 긴 바 하나가 아니라 **칸 여러 개**로 되어 있다. 체력이 줄면 오른쪽 칸부터 비고,
 * 걸친 칸은 그 칸 안에서 부분적으로 찬다. 칸은 코드가 자동으로 만들기 때문에
 * WBP 쪽에는 아무것도 배치하지 않아도 된다 (디자이너 비워둘 것).
 *
 * 최대 체력이 바뀌어도 칸 수는 그대로고, 칸 하나가 담는 체력만 달라진다.
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API UPlayerHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 체력을 직접 넣어 갱신 (BP에서 다른 대상의 체력을 보여주고 싶을 때)
	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void SetHealth(float Current, float Max);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// --- 모양 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar", meta=(ClampMin="1", ClampMax="30", ToolTip="칸 개수. 칸 하나가 담는 체력 = 최대 체력 ÷ 칸 수"))
	int32 SegmentCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar", meta=(ToolTip="칸 하나의 크기(px)"))
	FVector2D SegmentSize = FVector2D(40.f, 24.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar", meta=(ClampMin="0", ToolTip="칸 사이 간격(px)"))
	float SegmentGap = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar", meta=(ClampMin="0", ToolTip="칸 모서리 둥글기(px)"))
	float CornerRadius = 2.f;

	// --- 색 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar|Color", meta=(ToolTip="남아 있는 체력 색"))
	FLinearColor FillColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar|Color", meta=(ToolTip="빈 칸 색 (배경)"))
	FLinearColor EmptyColor = FLinearColor(0.f, 0.f, 0.f, 0.45f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar|Color", meta=(ToolTip="칸 테두리 색"))
	FLinearColor OutlineColor = FLinearColor(1.f, 1.f, 1.f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar|Color", meta=(ClampMin="0", ToolTip="칸 테두리 두께(px)"))
	float OutlineWidth = 1.f;

	// --- 연결 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar", meta=(ToolTip="true면 플레이어 캐릭터의 체력에 자동으로 연결된다"))
	bool bAutoBindToPlayer = true;

	// 코드가 만든 칸들
	UPROPERTY(Transient)
	TArray<UProgressBar*> Segments;

	UPROPERTY(Transient)
	UHorizontalBox* SegmentRow = nullptr;

	// 캐릭터의 OnHealthChanged에 연결되는 핸들러
	UFUNCTION()
	void HandleHealthChanged(float Current, float Max);

	void BuildSegments();
	void RefreshSegments();
	void BindToPlayer();

	TWeakObjectPtr<ACharacterBase> BoundCharacter;
	float CachedCurrent = 0.f;
	float CachedMax = 0.f;
};
