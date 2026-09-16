// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

class ACharacterBase;
class UWidgetAnimation;

// 적 머리 위 체력바 위젯의 C++ 베이스.
//
// WidgetComponent가 만든 위젯은 GetOwningPlayerPawn이 "플레이어"를 돌려주기 때문에
// WBP 안에서 자기를 띄운 적 캐릭터에 접근할 방법이 없다. 그래서 소유 캐릭터를
// C++이 직접 주입하고(BindToCharacter), 체력은 Tick 폴링이 아니라
// OnHealthChanged 델리게이트 구독으로 받는다.
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ACharacterBase::BeginPlay가 호출 — 소유자 주입 + OnHealthChanged 구독 + 초기값 반영
	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void BindToCharacter(ACharacterBase* InOwner);

	// WBP의 ProgressBar Percent 바인딩용
	UFUNCTION(BlueprintPure, Category = "HealthBar")
	float GetHealthPercent() const { return HealthPercent; }

	// 지연바(PB_Delayed) Percent 바인딩용 — 이전 체력에서 천천히 따라 내려온다.
	// PB_Health와의 차이 구간이 "방금 받은 대미지"로 보인다.
	UFUNCTION(BlueprintPure, Category = "HealthBar")
	float GetDelayedPercent() const { return DelayedPercent; }

	// 소유 캐릭터 사망 시 ACharacterBase가 호출 — WBP의 Anim_Death 재생.
	// BP에서 노드를 잇지 않아도 동작한다(BindWidgetAnim 자동 바인딩).
	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void PlayDeathEffect();

	UFUNCTION(BlueprintPure, Category = "HealthBar")
	ACharacterBase* GetOwnerCharacter() const { return OwnerCharacter; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar", meta=(ToolTip="피격 후 지연바가 제자리에 버티는 시간(초)"))
	float DelayedHoldTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthBar", meta=(ToolTip="지연바가 따라 내려오는 속도(초당 체력 비율)"))
	float DelayedDrainSpeed = 0.9f;

	UPROPERTY(BlueprintReadOnly, Category = "HealthBar", meta=(ToolTip="지연바 비율 0~1"))
	float DelayedPercent = 1.f;

	// WBP에 같은 이름("Anim_Death")의 위젯 애니메이션이 있으면 UMG가 자동으로 여기에 묶는다.
	// Optional이라 애니가 없는 WBP에서도 컴파일 에러가 나지 않는다.
	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	UWidgetAnimation* Anim_Death = nullptr;

	float DelayedHoldRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "HealthBar", meta=(ToolTip="이 체력바를 띄운 캐릭터. C++이 주입"))
	ACharacterBase* OwnerCharacter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "HealthBar", meta=(ToolTip="현재 체력 비율 0~1. ProgressBar Percent에 바인딩"))
	float HealthPercent = 1.f;

	// 값 갱신 직후 호출 — BP에서 지연바(흰 바) 애니, 피격 플래시 등 비주얼 처리
	UFUNCTION(BlueprintImplementableEvent, Category = "HealthBar")
	void OnHealthUpdated(float NewPercent, float OldPercent);

	UFUNCTION()
	void HandleHealthChanged(float Current, float Max);

	// 델리게이트 경로와 Tick 보정 경로가 공유하는 실제 갱신 지점
	void ApplyHealthPercent(float NewPercent);
};
