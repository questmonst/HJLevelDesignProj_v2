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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI", meta=(ToolTip="에디터에서 할당할 BehaviorTree 에셋"))
    UBehaviorTree* BehaviorTree;

    UPROPERTY(BlueprintReadOnly, Category = "Enemy|AI", meta=(ToolTip="경계 상태 여부. 피격·감지 시 true로 전환"))
    bool bIsAlerted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Enemy|AI", meta=(ToolTip="현재 타겟을 시야 내에서 확인 중인지"))
    bool bCanSeeTarget = false;

    // --- Combat ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta=(ToolTip="발사 1회당 피해량"))
    float AttackDamage = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta=(ToolTip="공격 가능 최대 거리 (cm)"))
    float AttackRange = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta=(ToolTip="공격 사이 쿨다운 (초)"))
    float AttackCooldown = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta=(ToolTip="BT가 유지하려는 전투 거리 (cm). 플레이어와의 이상적 거리"))
    float PreferredCombatRange = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta=(ToolTip="피격 시 이 반경 내 아군에게 알림 (cm)"))
    float AlertRadius = 1500.f;

    // --- Patrol ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Patrol", meta=(ToolTip="순찰 반경 (cm). PatrolOrigin 기준 이 범위 내에서 랜덤 이동"))
    float PatrolRadius = 500.f;

    // --- Weapon ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Weapon", meta=(ToolTip="스폰 시 자동으로 장착할 무기 클래스"))
    TSubclassOf<AWeaponBase> DefaultWeaponClass;

    UPROPERTY(BlueprintReadOnly, Category = "Enemy|Weapon", meta=(ToolTip="현재 장착된 무기 레퍼런스"))
    AWeaponBase* EnemyWeapon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Weapon", meta=(ToolTip="무기를 부착할 소켓 이름"))
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

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void FireAtTarget();

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void StopFiring();

    UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
    void AlertEnemy(AActor* Target);

    // --- Events (BP에서 오버라이드) ---

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Enemy|AI")
    void OnDetectPlayer();
    virtual void OnDetectPlayer_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Enemy|Combat")
    void OnAttack();
    virtual void OnAttack_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Enemy|AI")
    void OnLoseSight();
    virtual void OnLoseSight_Implementation();
};
