// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "CharacterBase.generated.h"

class UWidgetComponent;
class UUserWidget;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="마지막 피격 후 이 시간(초) 지나면 체력바 숨김"))
	float HealthBarHideDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="체력바 높이 오프셋 (캡슐 기준 위쪽 cm)"))
	float HealthBarHeightOffset = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|UI", meta=(ToolTip="이 거리(cm) 넘으면 체력바 숨김. 0=제한 없음"))
	float HealthBarMaxDrawDistance = 3000.f;

	bool bHealthBarActive = false;   // 피격 후 표시 상태 (거리 조건과 조합해 실제 가시성 결정)
	FTimerHandle HealthBarHideTimer;

	void ShowHealthBar();
	void HideHealthBar();            // 타이머 콜백: 표시 상태 해제
	void UpdateHealthBar(float DeltaTime);   // Tick: 빌보드 + 거리 가시성
	void SpawnDamageNumber(float Amount, const FVector& WorldLocation);

public:
	// WBP에서 프로그레스바 바인딩용
	UFUNCTION(BlueprintPure, Category = "Character|Stats")
	float GetHealthPercent() const;

	UPROPERTY(BlueprintAssignable, Category = "Character|Combat")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character|Combat")
	FOnDamagedSignature OnDamaged;

protected:
	// --- Internal ---

	void Die();

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
