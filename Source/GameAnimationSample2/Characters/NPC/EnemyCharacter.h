// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NPCCharacter.h"
#include "EnemyCharacter.generated.h"

class AWeaponBase;
class UBehaviorTree;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API AEnemyCharacter : public ANPCCharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

    // --- Blackboard 키 이름 (AEnemyAIController와 공유) ---
    static const FName BBKey_TargetActor;
    static const FName BBKey_TargetLocation;
    static const FName BBKey_bCanSeeTarget;
    static const FName BBKey_bIsAlerted;
    static const FName BBKey_PatrolOrigin;

protected:
    virtual void BeginPlay() override;

    // --- AI ---

    /** 에디터에서 할당할 BehaviorTree 에셋 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(BlueprintReadOnly, Category = "Enemy|AI")
    bool bIsAlerted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Enemy|AI")
    bool bCanSeeTarget = false;

    // --- Combat ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
    float AttackDamage = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
    float AttackRange = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
    float AttackCooldown = 0.8f;

    /** BT가 유지하려는 전투 거리 (플레이어와의 이상적 거리) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
    float PreferredCombatRange = 800.f;

    /** 피격 시 이 반경 내 아군에게 알림 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
    float AlertRadius = 1500.f;

    // --- Patrol ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Patrol")
    float PatrolRadius = 500.f;

    // --- Weapon ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Weapon")
    TSubclassOf<AWeaponBase> DefaultWeaponClass;

    UPROPERTY(BlueprintReadOnly, Category = "Enemy|Weapon")
    AWeaponBase* EnemyWeapon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Weapon")
    FName WeaponAttachSocket = TEXT("hand_r");

public:
    // --- Getters ---

    UFUNCTION(BlueprintPure, Category = "Enemy|AI")
    UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

    UFUNCTION(BlueprintPure, Category = "Enemy|AI")
    bool GetIsAlerted() const { return bIsAlerted; }

    UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
    float GetPreferredCombatRange() const { return PreferredCombatRange; }

    UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
    float GetAttackRange() const { return AttackRange; }

    // --- Combat ---

    /** BT Task에서 호출: 현재 타겟에게 발사 시작 */
    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void FireAtTarget();

    /** BT Task에서 호출: 발사 중지 */
    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void StopFiring();

    /** 피격/감지 시 호출: 경계 상태 전환 + 주변 아군 알림 */
    UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
    void AlertEnemy(AActor* Target);

    // --- Events (BP에서 오버라이드) ---

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Enemy|AI")
    void OnDetectPlayer();
    virtual void OnDetectPlayer_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Enemy|Combat")
    void OnAttack();
    virtual void OnAttack_Implementation();

    /** 시야를 잃었을 때 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Enemy|AI")
    void OnLoseSight();
    virtual void OnLoseSight_Implementation();
};
