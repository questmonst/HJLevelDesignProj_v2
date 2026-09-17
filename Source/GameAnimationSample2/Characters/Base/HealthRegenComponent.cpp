// Copyright Epic Games, Inc. All Rights Reserved.

#include "HealthRegenComponent.h"
#include "CharacterBase.h"
#include "TimerManager.h"

UHealthRegenComponent::UHealthRegenComponent()
{
	// 타이머로 동작하므로 틱 불필요
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthRegenComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacterBase>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerCharacter->OnDamaged.AddDynamic(this, &UHealthRegenComponent::HandleDamaged);
	}
}

void UHealthRegenComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->OnDamaged.RemoveDynamic(this, &UHealthRegenComponent::HandleDamaged);
	}
	GetWorld()->GetTimerManager().ClearTimer(DelayTimer);
	GetWorld()->GetTimerManager().ClearTimer(TickTimer);
	Super::EndPlay(EndPlayReason);
}

void UHealthRegenComponent::Configure(float InDelay, float InInterval, float InAmount, float InCapRatio)
{
	RegenDelay    = FMath::Max(InDelay, 0.f);
	RegenInterval = FMath::Max(InInterval, 0.01f);
	RegenAmount   = FMath::Max(InAmount, 0.f);
	RegenCapRatio = FMath::Clamp(InCapRatio, 0.f, 1.f);

	if (bIsRegenerating)
	{
		GetWorld()->GetTimerManager().SetTimer(TickTimer, this, &UHealthRegenComponent::RegenTick, RegenInterval, true);
	}
}

float UHealthRegenComponent::GetCapHealth() const
{
	return OwnerCharacter ? OwnerCharacter->GetMaxHealth() * RegenCapRatio : 0.f;
}

void UHealthRegenComponent::HandleDamaged(float Amount, FVector WorldLocation)
{
	// OnDamaged는 체력 감소 직전에 방송된다. 판단은 대기 시간이 끝난 시점의 체력으로 하므로 문제없다
	StopRegen();
	GetWorld()->GetTimerManager().SetTimer(DelayTimer, this, &UHealthRegenComponent::TryStartRegen, FMath::Max(RegenDelay, 0.01f), false);
}

void UHealthRegenComponent::TryStartRegen()
{
	if (!OwnerCharacter || OwnerCharacter->IsDead() || RegenAmount <= 0.f) return;
	if (OwnerCharacter->GetCurrentHealth() >= GetCapHealth()) return;   // 상한 위에서 맞았으면 회복 없음

	bIsRegenerating = true;
	OnRegenStarted.Broadcast();
	GetWorld()->GetTimerManager().SetTimer(TickTimer, this, &UHealthRegenComponent::RegenTick, RegenInterval, true);
}

void UHealthRegenComponent::RegenTick()
{
	if (!OwnerCharacter || OwnerCharacter->IsDead())
	{
		StopRegen();
		return;
	}

	const float Remaining = GetCapHealth() - OwnerCharacter->GetCurrentHealth();
	if (Remaining > 0.f)
	{
		OwnerCharacter->Heal(FMath::Min(RegenAmount, Remaining));
	}

	if (OwnerCharacter->GetCurrentHealth() >= GetCapHealth() - KINDA_SMALL_NUMBER)
	{
		StopRegen();
	}
}

void UHealthRegenComponent::StopRegen()
{
	GetWorld()->GetTimerManager().ClearTimer(TickTimer);
	if (!bIsRegenerating) return;

	bIsRegenerating = false;
	OnRegenStopped.Broadcast();
}
