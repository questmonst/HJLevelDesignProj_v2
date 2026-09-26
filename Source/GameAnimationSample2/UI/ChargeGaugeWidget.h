// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChargeGaugeWidget.generated.h"

class UImage;
class USizeBox;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class AMikaCharacter;

/**
 * 미카 펀치 충전 게이지 — 크로스헤어 아래쪽 반원 도넛.
 *
 * 왼쪽 끝(0) → 아래 → 오른쪽 끝(1)으로 차오른다. 눈금 전체가 **강제 발동 시간**
 * (ForcedMaxChargeTime, 현재 2초)이고, **최대 위력 지점**(MaxChargeTime, 1.5초)에 마커가 선다.
 * 최대 위력에 도달하면 채운 부분이 전부 흰색으로 바뀐다.
 *
 * 칸은 코드가 만들기 때문에 WBP 디자이너는 비워둬도 된다.
 * 크로스헤어에 얹을 때는 **사람이 직접 계층구조에 드래그 앤 드롭**할 것
 * (스크립트로 자식 위젯을 넣으면 런타임에 갱신되지 않는다).
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API UChargeGaugeWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

	// --- 모양 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeGauge", meta=(ToolTip="게이지 크기(px). 크로스헤어를 둘러쌀 만큼"))
	FVector2D GaugeSize = FVector2D(160.f, 160.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeGauge", meta=(ToolTip="반원 도넛 머티리얼 (M_UI_ChargeArc). Percent·Marker 스칼라와 색 벡터 파라미터 필요"))
	UMaterialInterface* ArcMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeGauge|Style", meta=(ToolTip="아직 안 찬 부분 색"))
	FLinearColor BackgroundColor = FLinearColor(0.f, 0.f, 0.f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeGauge|Style", meta=(ToolTip="채워진 부분 색 (최대 위력 전)"))
	FLinearColor FillColor = FLinearColor(0.55f, 0.75f, 1.f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeGauge|Style", meta=(ToolTip="최대 위력 도달 시 채운 부분 색"))
	FLinearColor FullColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeGauge|Style", meta=(ToolTip="최대 위력 지점 마커 색"))
	FLinearColor MarkerColor = FLinearColor(1.f, 0.85f, 0.3f, 1.f);

	// --- 코드가 만드는 위젯 ---

	UPROPERTY(Transient) USizeBox* GaugeBox   = nullptr;
	UPROPERTY(Transient) UImage*   ArcImage   = nullptr;
	UPROPERTY(Transient) UMaterialInstanceDynamic* ArcMID = nullptr;

	void BuildLayout();
	// 디자이너 설정값을 실제 위젯에 다시 적용 — 런타임 인스턴스는 머티리얼 인스턴스를 새로 만들어야 한다
	void RefreshVisualsFromSettings();

	TWeakObjectPtr<AMikaCharacter> Mika;
};
