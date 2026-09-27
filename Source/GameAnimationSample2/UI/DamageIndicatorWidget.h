// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageIndicatorWidget.generated.h"

class UImage;
class UOverlay;
class UTexture2D;

/**
 * 피격 방향 표시 — 플레이어가 맞으면 화면 중앙 둘레에 공격이 날아온 쪽을 가리키는 표시가 뜬다.
 *
 * 표시는 월드 위치를 기억해서, 맞은 뒤 몸을 돌려도 계속 그 방향을 가리킨다.
 * 같은 방향에서 연달아 맞으면 새로 띄우지 않고 기존 표시를 갱신한다.
 *
 * 칸은 코드가 만들기 때문에 WBP 디자이너는 비워둬도 된다.
 * 크로스헤어에 얹을 때는 사람이 직접 계층구조에 드래그 앤 드롭하고, 화면 중앙에 두고 크기를 충분히 크게
 * (표시 반경 × 2 이상) 잡을 것.
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API UDamageIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

	// --- 모양 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageIndicator", meta=(ToolTip="표시 이미지. 화면 위쪽(=공격 방향)을 가리키는 모양으로 그릴 것 (쐐기·호 등). 비우면 둥근 막대"))
	UTexture2D* IndicatorTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageIndicator", meta=(ToolTip="표시 하나의 크기(px). 가로가 둘레 방향"))
	FVector2D IndicatorSize = FVector2D(90.f, 14.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageIndicator", meta=(ClampMin="0", ToolTip="화면 중앙에서 표시까지 거리(px)"))
	float Radius = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageIndicator", meta=(ToolTip="표시 색"))
	FLinearColor Color = FLinearColor(1.f, 0.15f, 0.1f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageIndicator", meta=(ClampMin="0.1", ToolTip="표시가 떠 있는 시간(초)"))
	float Duration = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageIndicator", meta=(ClampMin="0", ClampMax="1", ToolTip="이 비율부터 흐려지기 시작 (0.5 = 절반 지나서부터)"))
	float FadeStartRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageIndicator", meta=(ClampMin="1", ClampMax="16", ToolTip="동시에 띄울 수 있는 최대 개수"))
	int32 MaxIndicators = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageIndicator", meta=(ClampMin="0", ClampMax="180", ToolTip="이 각도(도) 안쪽에서 또 맞으면 새로 띄우지 않고 기존 표시를 갱신"))
	float MergeAngle = 25.f;

	// --- 코드가 만드는 위젯 ---

	UPROPERTY(Transient) UOverlay* IndicatorRoot = nullptr;
	UPROPERTY(Transient) TArray<UImage*> Images;

	struct FIndicator
	{
		FVector Source = FVector::ZeroVector;   // 공격이 날아온 월드 위치
		float   TimeLeft = 0.f;
	};
	TArray<FIndicator> Indicators;   // Images와 같은 인덱스

	void BuildLayout();
	void ApplyBrushes();

	// 월드 위치 → 화면 중앙 기준 표시 각도(도, 12시 = 0, 시계방향 +).
	// 카메라 앞이면 화면에 보이는 적 쪽을, 뒤면 수평 방향을 가리킨다
	float GetRelativeYaw(const FVector& Source) const;

	UFUNCTION()
	void HandleDamagedFrom(float Amount, FVector SourceLocation);
};
