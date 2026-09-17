// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "CharacterBase.generated.h"

class UWidgetComponent;
class UUserWidget;
class UEnemyHealthBarWidget;
class ADamageNumberActor;

// WBP·다른 시스템이 바인딩할 수 있는 체력/피격 이벤트 (SSOT: 체력은 CurrentHealth 하나)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamagedSignature, float, Amount, FVector, WorldLocation);

UCLASS(Blueprintable, BlueprintType, meta=(PrioritizeCategories="Character|Stats"))
class GAMEANIMATIONSAMPLE2_API ACharacterBase : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ACharacterBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats", meta=(ToolTip="최대 체력"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats", meta=(ToolTip="현재 체력. BeginPlay에서 MaxHealth로 초기화"))
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stats", meta=(ToolTip="팀 ID. 0=플레이어·아군, 1=적. AIPerception 팀 감지에 사용"))
	int32 TeamID;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Stats", meta=(ToolTip="사망 여부. true면 OnDeath 이후 상태"))
	bool bIsDead = false;

	// --- Floating Health Bar / Damage Number ---
	// 로직·데이터는 여기(C++), 비주얼(WBP)은 BP에서. 기본은 꺼짐(false)이고 적 클래스에서 켠다.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|UI", meta=(ToolTip="머리 위 체력바 위젯 컴포넌트 (World 스페이스)"))
	UWidgetComponent* HealthBarWidgetComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="머리 위 체력바 표시 여부. 적 클래스에서 기본 true"))
	bool bShowFloatingHealthBar = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="체력바로 사용할 위젯 클래스 (WBP_EnemyHealthBar 등)"))
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="피격 시 스폰할 대미지 숫자 액터 클래스 (BP_DamageNumber)"))
	TSubclassOf<ADamageNumberActor> DamageNumberActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="마지막 피격 후 이 시간(초) 지나면 체력바 페이드 아웃 시작"))
	float HealthBarHideDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="체력바가 완전히 사라지기까지 걸리는 페이드 시간(초). 0=즉시"))
	float HealthBarFadeDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="사망 시 연출(체력바 확대·소멸) 재생 후 액터를 실제 제거하기까지의 시간(초)"))
	float DeathEffectDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="체력바 높이 오프셋 (캡슐 기준 위쪽 cm)"))
	float HealthBarHeightOffset = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="이 거리(cm) 넘으면 체력바 숨김. 0=제한 없음"))
	float HealthBarMaxDrawDistance = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="이 거리(cm)에서 체력바가 기준 크기(1.0). 가까워도 더 커지진 않는다. 0=거리 보정 안 함"))
	float HealthBarRefDistance = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="멀어져도 이 배율 아래로는 안 줄어듦(가독성 하한). 1.0=거리와 무관하게 항상 같은 크기"))
	float HealthBarMinScale = 0.6f;

	bool bHealthBarActive = false;   // 피격 후 표시 상태 (거리·화면 조건과 조합해 실제 가시성 결정)
	float HealthBarAlpha = 0.f;      // 현재 페이드 알파 0~1. 위젯 RenderOpacity에 반영
	FTimerHandle HealthBarHideTimer;
	FTimerHandle DeathDestroyTimer;  // 사망 연출 후 실제 Destroy 예약

	void ShowHealthBar();
	void HideHealthBar();            // 타이머 콜백: 표시 상태 해제
	void UpdateHealthBar(float DeltaTime);   // Tick: 빌보드 + 거리 가시성 + 페이드 + 최소 크기 보정
	void ApplyHealthBarAlpha(float Alpha);   // 알파를 위젯 RenderOpacity에 반영 (0이면 렌더에서 제외)
	void SpawnDamageNumber(float Amount, const FVector& WorldLocation);

public:
	// WBP에서 프로그레스바 바인딩용
	UFUNCTION(BlueprintPure, Category = "Character|Stats")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Character|Stats")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Character|Stats")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Character|Stats")
	bool IsDead() const { return bIsDead; }

	// 체력 회복의 유일한 경로 — 회복 컴포넌트·아이템 등이 CurrentHealth를 직접 바꾸지 않게 해서
	// OnHealthChanged 방송이 누락되지 않도록 한다 (SSOT). 실제 회복된 양을 반환
	UFUNCTION(BlueprintCallable, Category = "Character|Stats")
	float Heal(float Amount);

	UPROPERTY(BlueprintAssignable, Category = "Character|Combat")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character|Combat")
	FOnDamagedSignature OnDamaged;

	// 사망 순간 호출 — BP에서 체력바 위젯의 사망 연출(확대·소멸) 애니를 재생.
	// 액터는 DeathEffectDuration 후 실제 제거되므로 연출 재생 시간이 확보된다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Combat")
	void OnDeathEffect();

protected:
	// --- Internal ---

	void Die();
	void FinishDeath();   // 사망 연출 후(타이머 콜백) 실제 Destroy

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	                         AController* EventInstigator, AActor* DamageCauser) override;

public:
	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(TeamID); }

	// --- BlueprintNativeEvent ---

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character|Combat")
	void OnDeath();
	virtual void OnDeath_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character|Combat")
	void TakeDamageCustom(float Amount);
	virtual void TakeDamageCustom_Implementation(float Amount);
};
