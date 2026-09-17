// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthRegenComponent.generated.h"

class ACharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRegenStateChangedSignature);

// 일정 시간 피격이 없으면 체력을 상한 비율까지 N초마다 M씩 회복시키는 컴포넌트.
//
// 캐릭터 클래스에 직접 넣지 않고 컴포넌트로 둔 이유: PlayerCharacter가 이미 500줄 한도에 가깝고,
// 미유·특정 적에게도 같은 규칙을 붙일 수 있게 하기 위해서다.
// 체력은 직접 만지지 않고 ACharacterBase::Heal만 호출한다 — 체력의 진실은 캐릭터 하나(SSOT).
// 회복 시작/종료를 델리게이트로 알려 체력바 깜빡임 같은 연출을 UI가 알아서 붙이게 한다.
UCLASS(ClassGroup=(Character), meta=(BlueprintSpawnableComponent))
class GAMEANIMATIONSAMPLE2_API UHealthRegenComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthRegenComponent();

	// DataAsset 값 주입용 (MikaCharacter::BeginPlay). 값이 바뀌면 진행 중인 회복도 새 간격으로 재시작
	void Configure(float InDelay, float InInterval, float InAmount, float InCapRatio);

	UFUNCTION(BlueprintPure, Category = "Regen")
	bool IsRegenerating() const { return bIsRegenerating; }

	// 회복이 시작될 때 (체력바 깜빡임 재생)
	UPROPERTY(BlueprintAssignable, Category = "Regen")
	FOnRegenStateChangedSignature OnRegenStarted;

	// 회복이 끝날 때 — 상한 도달, 피격으로 중단, 사망 모두 포함 (깜빡임 정지)
	UPROPERTY(BlueprintAssignable, Category = "Regen")
	FOnRegenStateChangedSignature OnRegenStopped;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Regen", meta=(ClampMin="0", ToolTip="마지막 피격 후 회복 시작까지 대기 시간 (초). DataAsset에서 설정"))
	float RegenDelay = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Regen", meta=(ClampMin="0.01", ToolTip="회복 간격 N (초). DataAsset에서 설정"))
	float RegenInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Regen", meta=(ClampMin="0", ToolTip="간격마다 회복량 M. DataAsset에서 설정"))
	float RegenAmount = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Regen", meta=(ClampMin="0", ClampMax="1", ToolTip="회복 상한 (최대 체력 대비 비율). 이 위에서는 회복하지 않음. DataAsset에서 설정"))
	float RegenCapRatio = 0.5f;

private:
	UFUNCTION()
	void HandleDamaged(float Amount, FVector WorldLocation);

	void TryStartRegen();
	void RegenTick();
	void StopRegen();
	float GetCapHealth() const;

	UPROPERTY()
	TObjectPtr<ACharacterBase> OwnerCharacter = nullptr;

	bool bIsRegenerating = false;
	FTimerHandle DelayTimer;
	FTimerHandle TickTimer;
};
