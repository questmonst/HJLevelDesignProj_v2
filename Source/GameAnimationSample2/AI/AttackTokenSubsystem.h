// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Subsystems/WorldSubsystem.h"
#include "AttackTokenSubsystem.generated.h"

class AEnemyCharacter;
class USoundBase;

// 공격 토큰 규칙 (DA_CombatDirector). MikaData › Combat에 지정한다.
//
// 적은 계속 쏘지만 "맞는 탄"을 쏘는 적은 동시에 MaxTokens명뿐이다.
// 토큰을 받은 적만 레이저로 예고한 뒤 정확히 쏘고, 나머지는 일부러 빗나가게 쏜다 (플레이어에게 절대 안 맞는다).
UCLASS(BlueprintType)
class GAMEANIMATIONSAMPLE2_API UCombatDirectorData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Token", meta=(ClampMin="0", ToolTip="동시에 명중 사격을 할 수 있는 적 수. 0이면 아무도 못 맞힌다"))
	int32 MaxTokens = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Token", meta=(ClampMin="0", ToolTip="토큰을 받고 레이저를 켠 뒤 첫 명중탄까지 시간(초). 플레이어가 피하거나 엄폐할 여유"))
	float TelegraphTime = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Token", meta=(ClampMin="0", ToolTip="같은 적이 토큰을 다시 받기까지 시간(초). 적이 여럿이면 돌아가며 받게 된다"))
	float TokenCooldown = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Token", meta=(ClampMin="0", ToolTip="명중 버스트가 끝난 뒤 다음 토큰을 주기까지 최소 간격(초). 숨 돌릴 틈"))
	float GlobalGap = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Token", meta=(ToolTip="토큰을 받는 순간(레이저가 켜질 때) 그 적 위치에서 나는 경고음"))
	USoundBase* TelegraphSound = nullptr;

	// --- 빗나가는 탄 ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Miss", meta=(ClampMin="0", ToolTip="토큰 없는 적이 겨누는 점이 플레이어에서 떨어진 최소 거리(cm)"))
	float MissOffsetMin = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Miss", meta=(ClampMin="0", ToolTip="토큰 없는 적이 겨누는 점이 플레이어에서 떨어진 최대 거리(cm)"))
	float MissOffsetMax = 150.f;

	// --- 누가 먼저 받나 ---
	// 점수 = 적 가중치(EnemyData TokenWeight) × 화면 안 보정 × (1 + 기다린 시간 × 증가율) ÷ 거리

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Priority", meta=(ClampMin="1", ToolTip="플레이어 화면 안에 있는 적의 점수 배율. 안 보이는 곳에서 맞는 억울함을 줄인다"))
	float OnScreenMultiplier = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Priority", meta=(ClampMin="0", ClampMax="90", ToolTip="화면 안으로 치는 각도(카메라 정면 기준 좌우, 도)"))
	float OnScreenHalfAngle = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Priority", meta=(ClampMin="0", ToolTip="토큰을 못 받고 기다린 1초당 점수 증가율. 0.2면 5초 기다릴 때 2배"))
	float WaitGrowthPerSecond = 0.2f;
};

// 공격 토큰 관리자 — 사격 중인 적(후보) 가운데 점수가 높은 순으로 토큰을 나눠준다.
// 토큰은 Fire At Target 태스크가 명중 버스트를 끝내면 반납한다. 죽거나 쓰러지면 자동 회수.
UCLASS()
class GAMEANIMATIONSAMPLE2_API UAttackTokenSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UAttackTokenSubsystem, STATGROUP_Tickables); }

	// 규칙 DA 지정 (플레이어가 BeginPlay에서). 없으면 클래스 기본값
	void SetData(const UCombatDirectorData* InData);
	const UCombatDirectorData& GetData() const;

	// Fire At Target 태스크가 사격을 시작/끝낼 때 — 후보만 토큰을 받을 수 있다
	void RegisterCandidate(AEnemyCharacter* Enemy);
	void UnregisterCandidate(AEnemyCharacter* Enemy);

	// 명중 버스트가 끝났을 때 (또는 태스크가 중단됐을 때)
	void ReleaseToken(AEnemyCharacter* Enemy);

	// 클라이맥스 연출용 — 토큰 수를 런타임에 바꾼다 (음수면 DA 값으로 복귀)
	UFUNCTION(BlueprintCallable, Category = "Combat|Token")
	void SetMaxTokensOverride(int32 NewMax) { MaxTokensOverride = NewMax; }

	UFUNCTION(BlueprintPure, Category = "Combat|Token")
	int32 GetMaxTokens() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<const UCombatDirectorData> Data;

	TArray<TWeakObjectPtr<AEnemyCharacter>> Candidates;
	TArray<TWeakObjectPtr<AEnemyCharacter>> Holders;

	// 적별 기록: 마지막으로 토큰을 반납한 시각, 후보가 된 시각(기다린 시간 계산)
	TMap<TWeakObjectPtr<AEnemyCharacter>, float> LastReleaseTime;
	TMap<TWeakObjectPtr<AEnemyCharacter>, float> WaitingSince;

	float LastAnyReleaseTime = -1000.f;
	int32 MaxTokensOverride  = -1;

	float ScoreCandidate(const AEnemyCharacter* Enemy, float Now) const;
	static bool CanHoldToken(const AEnemyCharacter* Enemy);
};
