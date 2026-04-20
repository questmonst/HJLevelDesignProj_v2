// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NPCCharacter.h"
#include "MiyuCharacter.generated.h"

class APlayerCharacter;

/**
 * 미유 — 지원사격 NPC
 *
 * 사용법 (레벨 BP / GameMode):
 *   1. MiyuCharacter 배치
 *   2. SetTarget(AkiraRef) 호출 → 자동으로 조건 체크 시작
 *   3. 조건 만족 시 OnSupportFire 발동 → BP에서 몽타주 + 발사체 처리
 *   4. ClearTarget() 호출 시 사격 중지
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

	// ────────────────────────────────────────────────
	// Config
	// ────────────────────────────────────────────────

	/** 사격 가능 최대 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miyu|Support")
	float SupportFireRange = 1500.f;

	/** 발사 쿨다운 (초) — 이 주기로 조건을 재확인하고 발사 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miyu|Support")
	float FireCooldown = 2.f;

	/** 첫 발사까지 대기 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miyu|Support")
	float FirstFireDelay = 0.5f;

	// ────────────────────────────────────────────────
	// State (BlueprintReadOnly — AnimBP, BT에서 읽기용)
	// ────────────────────────────────────────────────

	/** 현재 사격 타겟 (보통 아키라) */
	UPROPERTY(BlueprintReadOnly, Category = "Miyu|Support")
	AActor* CurrentTarget = nullptr;

	/** 사격 타이머가 돌아가는 중인지 */
	UPROPERTY(BlueprintReadOnly, Category = "Miyu|Support")
	bool bIsFiring = false;

	/** 캐시된 플레이어 레퍼런스 (LoS 차단 판정용) */
	UPROPERTY(BlueprintReadOnly, Category = "Miyu|Support")
	APlayerCharacter* PlayerRef = nullptr;

private:
	FTimerHandle FireTimerHandle;

	/** 타이머 콜백: 조건 체크 후 OnSupportFire 발동 */
	void TryFire();

public:
	// ────────────────────────────────────────────────
	// Target Control
	// ────────────────────────────────────────────────

	/**
	 * 사격 타겟 지정. 레벨 BP 또는 GameMode에서 호출.
	 * 호출 즉시 FirstFireDelay 후 첫 조건 체크 시작.
	 */
	UFUNCTION(BlueprintCallable, Category = "Miyu|Support")
	void SetTarget(AActor* NewTarget);

	/** 사격 중지 및 타겟 해제. */
	UFUNCTION(BlueprintCallable, Category = "Miyu|Support")
	void ClearTarget();

	// ────────────────────────────────────────────────
	// Query
	// ────────────────────────────────────────────────

	/**
	 * 지금 당장 발사할 수 있는지 확인.
	 * (거리 + LoS + 플레이어 미차단 모두 통과해야 true)
	 */
	UFUNCTION(BlueprintPure, Category = "Miyu|Support")
	bool CanFireAtTarget() const;

	/**
	 * Miyu → Target 사이 LineTrace.
	 * 지오메트리 또는 Player가 막고 있으면 false.
	 */
	UFUNCTION(BlueprintCallable, Category = "Miyu|Support")
	bool HasLineOfSightToTarget(const AActor* Target) const;

	// ────────────────────────────────────────────────
	// Events (BP에서 오버라이드)
	// ────────────────────────────────────────────────

	/**
	 * 사격 조건 충족 시 호출.
	 * BP에서: 사격 몽타주 재생 + 발사체(또는 히트스캔) 처리.
	 * @param Target  현재 사격 대상
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Miyu|Support")
	void OnSupportFire(AActor* Target);
	virtual void OnSupportFire_Implementation(AActor* Target);

	/**
	 * SetTarget()으로 타겟이 지정됐을 때.
	 * BP에서: 경계 애니메이션, UI 등.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Miyu|Support")
	void OnTargetAcquired(AActor* Target);
	virtual void OnTargetAcquired_Implementation(AActor* Target);

	/**
	 * ClearTarget() 또는 타겟 소멸 시.
	 * BP에서: 대기 애니메이션 복귀 등.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Miyu|Support")
	void OnTargetLost();
	virtual void OnTargetLost_Implementation();
};
