// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyHealthBarWidget.h"
#include "CharacterBase.h"
#include "Animation/WidgetAnimation.h"

void UEnemyHealthBarWidget::BindToCharacter(ACharacterBase* InOwner)
{
	if (!InOwner) return;

	// 위젯 컴포넌트가 위젯을 재생성하는 경우를 대비해 이전 구독을 먼저 정리
	if (OwnerCharacter)
	{
		OwnerCharacter->OnHealthChanged.RemoveDynamic(this, &UEnemyHealthBarWidget::HandleHealthChanged);
	}

	const bool bSameOwner = (OwnerCharacter == InOwner);

	OwnerCharacter = InOwner;
	OwnerCharacter->OnHealthChanged.AddDynamic(this, &UEnemyHealthBarWidget::HandleHealthChanged);

	// 구독 전에 이미 깎였을 수 있으므로 현재 값으로 한 번 맞춰준다
	HealthPercent = OwnerCharacter->GetHealthPercent();

	// 지연바는 처음 붙을 때만 현재 체력에 맞춘다. 재바인딩(위젯 재구성) 때마다 리셋하면
	// 연속 피격 중 지연바가 매번 따라잡혀 대미지 구간이 사라진다.
	DelayedPercent = bSameOwner ? FMath::Max(DelayedPercent, HealthPercent) : HealthPercent;

	OnHealthUpdated(HealthPercent, HealthPercent);
}

void UEnemyHealthBarWidget::HandleHealthChanged(float Current, float Max)
{
	ApplyHealthPercent(Max > 0.f ? Current / Max : 0.f);
}

void UEnemyHealthBarWidget::ApplyHealthPercent(float NewPercent)
{
	const float Old = HealthPercent;
	HealthPercent = FMath::Clamp(NewPercent, 0.f, 1.f);

	// 피해를 입었을 때만 지연바를 붙잡아 둔다 — 그 차이가 대미지 양으로 보인다.
	if (HealthPercent < Old)
	{
		DelayedHoldRemaining = DelayedHoldTime;
	}

	OnHealthUpdated(HealthPercent, Old);
}

void UEnemyHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// WidgetComponent는 숨겨질 때 슬레이트 위젯을 해제하고 다시 보일 때 재구성한다.
	// 그 사이 NativeDestruct가 구독을 끊어놨으므로, 기억해둔 소유자로 다시 묶는다.
	// (이게 없으면 바가 한 번 사라진 뒤의 피격이 반영되지 않는다)
	if (OwnerCharacter)
	{
		BindToCharacter(OwnerCharacter);
	}
}

void UEnemyHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 자가 보정 — 위젯이 통째로 새로 생성되는 등으로 델리게이트를 놓쳐도
	// 표시 값이 실제 체력과 어긋난 채 남지 않게 한다. (표시 중일 때만 도는 경로)
	if (OwnerCharacter)
	{
		const float Actual = OwnerCharacter->GetHealthPercent();
		if (!FMath::IsNearlyEqual(Actual, HealthPercent))
		{
			ApplyHealthPercent(Actual);
		}
	}

	if (DelayedPercent > HealthPercent)
	{
		// 잠깐 버틴 뒤 현재 체력까지 일정 속도로 따라 내려온다
		if (DelayedHoldRemaining > 0.f)
		{
			DelayedHoldRemaining -= InDeltaTime;
		}
		else
		{
			DelayedPercent = FMath::Max(
				HealthPercent, DelayedPercent - DelayedDrainSpeed * InDeltaTime);
		}
	}
	else if (DelayedPercent < HealthPercent)
	{
		DelayedPercent = HealthPercent;   // 회복은 지연 없이 즉시 따라붙는다
	}
}

void UEnemyHealthBarWidget::PlayDeathEffect()
{
	if (Anim_Death)
	{
		PlayAnimation(Anim_Death);
	}
}

void UEnemyHealthBarWidget::NativeDestruct()
{
	// 구독만 끊고 OwnerCharacter는 유지한다 — 위젯이 재구성될 때
	// NativeConstruct에서 다시 묶으려면 소유자를 기억하고 있어야 한다.
	if (OwnerCharacter)
	{
		OwnerCharacter->OnHealthChanged.RemoveDynamic(this, &UEnemyHealthBarWidget::HandleHealthChanged);
	}
	Super::NativeDestruct();
}
