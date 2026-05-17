// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NPCCharacter.h"
#include "MiyuCharacter.generated.h"

class APlayerCharacter;

/**
 * 미유 — 지원사격 NPC
 *
 * 사격 조건 (모두 충족해야 발사):
 *   - Target이 SupportFireRange 이내
 *   - Miyu → Target 사이 지오메트리 없음
 *   - Player가 Miyu와 Target 사이에 없음 (플레이어가 사격각 확보 필요)
 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AMiyuCharacter : public ANPCCharacter
{
	GENERATED_BODY()

public:
	AMiyuCharacter();

protected:
	virtual void BeginPlay() override;

	// --- Config ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miyu|Support", meta=(ToolTip="사격 가능 최대 거리 (cm)"))
	float SupportFireRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miyu|Support", meta=(ToolTip="발사 쿨다운 (초). 이 주기로 조건을 재확인하고 발사"))
	float FireCooldown = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miyu|Support", meta=(ToolTip="첫 발사까지 대기 시간 (초)"))
	float FirstFireDelay = 0.5f;

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "Miyu|Support", meta=(ToolTip="현재 사격 타겟 (보통 아키라)"))
	AActor* CurrentTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Miyu|Support", meta=(ToolTip="사격 타이머가 돌아가는 중인지"))
	bool bIsFiring = false;

	UPROPERTY(BlueprintReadOnly, Category = "Miyu|Support", meta=(ToolTip="캐시된 플레이어 레퍼런스 (사격각 판정용)"))
	APlayerCharacter* PlayerRef = nullptr;

private:
	FTimerHandle FireTimerHandle;

	void TryFire();

public:
	UFUNCTION(BlueprintCallable, Category = "Miyu|Support")
	void SetTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Miyu|Support")
	void ClearTarget();

	UFUNCTION(BlueprintPure, Category = "Miyu|Support")
	bool CanFireAtTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Miyu|Support")
	bool HasLineOfSightToTarget(const AActor* Target) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Miyu|Support")
	void OnSupportFire(AActor* Target);
	virtual void OnSupportFire_Implementation(AActor* Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Miyu|Support")
	void OnTargetAcquired(AActor* Target);
	virtual void OnTargetAcquired_Implementation(AActor* Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Miyu|Support")
	void OnTargetLost();
	virtual void OnTargetLost_Implementation();
};
