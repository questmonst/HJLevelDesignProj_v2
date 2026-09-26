// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillIconWidget.generated.h"

class UImage;
class UOverlay;
class USizeBox;
class UTextBlock;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTexture2D;
class AMikaCharacter;

// 이 아이콘이 무엇을 보여줄지
UENUM(BlueprintType)
enum class ESkillSlot : uint8
{
	Punch    UMETA(DisplayName = "미카 펀치"),
	Grenade  UMETA(DisplayName = "수류탄"),
	Dodge    UMETA(DisplayName = "회피"),
	Custom   UMETA(DisplayName = "직접 갱신")   // SetCooldown()을 직접 불러서 쓴다
};

/**
 * 오버워치식 스킬 쿨타임 아이콘.
 *
 * 평소에는 회백색 아이콘만 보이고, 쿨타임 중에는 그 위에 어두운 회색 덮개가
 * **시계방향으로 차오르며** 남은 시간을 보여준다. 쿨타임이 끝나면 한 번 번쩍인다.
 *
 * 칸 구성은 코드가 만들기 때문에 WBP 디자이너는 비워둬도 된다.
 */
UCLASS()
class GAMEANIMATIONSAMPLE2_API USkillIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 직접 갱신용 (ESkillSlot::Custom). Remaining이 0이면 준비 완료로 본다
	UFUNCTION(BlueprintCallable, Category = "SkillIcon")
	void SetCooldown(float Remaining, float Duration);

	UFUNCTION(BlueprintCallable, Category = "SkillIcon", meta=(ToolTip="아이콘 오른쪽 아래 숫자 (수류탄 개수 등). 음수면 숨김"))
	void SetCount(int32 Count);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

	// --- 무엇을 보여줄지 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillIcon", meta=(ToolTip="표시할 스킬. Custom이면 SetCooldown()으로 직접 갱신"))
	ESkillSlot SkillType = ESkillSlot::Punch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillIcon", meta=(ToolTip="아이콘 텍스처. 비우면 단색 사각형"))
	UTexture2D* IconTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillIcon", meta=(ToolTip="아이콘 아래 표시할 키 안내 (예: 마우스 우클릭). 비우면 숨김"))
	FText KeyLabel;

	// --- 모양 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillIcon|Style", meta=(ToolTip="아이콘 크기(px)"))
	FVector2D IconSize = FVector2D(64.f, 64.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillIcon|Style", meta=(ToolTip="평소 아이콘 색 (회백색)"))
	FLinearColor ReadyTint = FLinearColor(0.85f, 0.87f, 0.88f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillIcon|Style", meta=(ToolTip="쿨타임 중 아이콘 색 (어둡게)"))
	FLinearColor CooldownTint = FLinearColor(0.35f, 0.36f, 0.38f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillIcon|Style", meta=(ToolTip="시계방향으로 차오르는 덮개 색"))
	FLinearColor SweepColor = FLinearColor(0.1f, 0.1f, 0.12f, 0.75f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillIcon|Style", meta=(ToolTip="쿨타임 완료 시 번쩍이는 색"))
	FLinearColor FlashColor = FLinearColor(1.f, 1.f, 1.f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillIcon|Style", meta=(ClampMin="0", ToolTip="번쩍임 지속 시간(초)"))
	float FlashDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillIcon|Style", meta=(ToolTip="시계방향 덮개용 머티리얼 (M_UI_RadialCooldown). Percent 스칼라와 SweepColor 벡터 파라미터 필요"))
	UMaterialInterface* SweepMaterial = nullptr;

	// --- 코드가 만드는 위젯들 ---

	UPROPERTY(Transient) USizeBox*   IconBox    = nullptr;
	UPROPERTY(Transient) UOverlay*   IconLayers = nullptr;
	UPROPERTY(Transient) UImage*     IconImage  = nullptr;
	UPROPERTY(Transient) UImage*     SweepImage = nullptr;
	UPROPERTY(Transient) UImage*     FlashImage = nullptr;
	UPROPERTY(Transient) UTextBlock* CountText  = nullptr;
	UPROPERTY(Transient) UMaterialInstanceDynamic* SweepMID = nullptr;

	void BuildLayout();
	// 디자이너 설정값(아이콘·색·머티리얼)을 실제 위젯에 다시 적용 — 런타임 인스턴스용
	void RefreshVisualsFromSettings();
	void ApplyCooldownVisual(float Remaining, float Duration);
	void ReadFromPlayer();

	TWeakObjectPtr<AMikaCharacter> Mika;
	float FlashTimeLeft = 0.f;
	bool  bWasReady = true;
};
