// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NPCCharacter.h"
#include "EnemyCharacter.generated.h"

class AWeaponBase;
class UBehaviorTree;
class UEnemyDataAsset;
class UAnimMontage;

// BT의 Set Move Mode 태스크가 고르는 이동 자세. 속도 수치는 캐릭터(EnemyData)가 들고 있다.
UENUM(BlueprintType)
enum class EEnemyMoveMode : uint8
{
    Walk   UMETA(DisplayName = "Walk"),
    Run    UMETA(DisplayName = "Run"),
    Crouch UMETA(DisplayName = "Crouch"),
};

// BT의 Fire At Target이 EnemyData에서 읽을 사격/휴식 시간 묶음
UENUM(BlueprintType)
enum class EEnemyFirePattern : uint8
{
    Attack   UMETA(DisplayName = "Attack (일반 사격)"),
    Suppress UMETA(DisplayName = "Suppress (제압 사격)"),
};

// BT의 Random Chance가 EnemyData에서 읽을 확률 항목
UENUM(BlueprintType)
enum class EEnemyBehaviorChance : uint8
{
    Strafe    UMETA(DisplayName = "Strafe"),
    Suppress  UMETA(DisplayName = "Suppress"),
    SeekCover UMETA(DisplayName = "Seek Cover"),
};

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
    static const FName BBKey_bNeedsReload;

protected:
    virtual void BeginPlay() override;

    // 피격 시 공격자 방향을 컨트롤러에 알려 "그쪽을 보게" 한다
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    // DataAsset → 멤버 변수 복사. Super::BeginPlay 전에 불러야 MaxHealth가 체력 초기화에 반영된다
    void ApplyEnemyData();

    // --- Data ---

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Data", meta=(ToolTip="적 종류별 수치 에셋 (DA_Enemy_AR 등). 비우면 C++ 생성자 기본값 사용"))
    UEnemyDataAsset* EnemyData = nullptr;

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

    // --- Movement ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement", meta=(ToolTip="걷기 속도 (cm/s). EnemyData에서 설정"))
    float WalkSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement", meta=(ToolTip="달리기 속도 (cm/s). EnemyData에서 설정"))
    float RunSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement", meta=(ToolTip="앉아서 이동하는 속도 (cm/s). EnemyData에서 설정"))
    float CrouchWalkSpeed = 150.f;

    UPROPERTY(BlueprintReadOnly, Category = "Enemy|Movement", meta=(ToolTip="현재 이동 자세"))
    EEnemyMoveMode MoveMode = EEnemyMoveMode::Walk;

    // --- Fire Pattern ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Fire Pattern", meta=(ToolTip="일반 사격 유지 시간 (초). EnemyData에서 설정"))
    float FireBurstDuration = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Fire Pattern", meta=(ToolTip="일반 사격 후 휴식 (초). EnemyData에서 설정"))
    float FireRestDuration = 4.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Fire Pattern", meta=(ToolTip="제압 사격 유지 시간 (초). EnemyData에서 설정"))
    float SuppressBurstDuration = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Fire Pattern", meta=(ToolTip="제압 사격 후 휴식 (초). EnemyData에서 설정"))
    float SuppressRestDuration = 3.f;

    // --- Behavior Chance ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Behavior Chance", meta=(ToolTip="스트레이핑 확률 (%). EnemyData에서 설정"))
    float StrafeChance = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Behavior Chance", meta=(ToolTip="제압 사격 확률 (%). EnemyData에서 설정"))
    float SuppressChance = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Behavior Chance", meta=(ToolTip="개활지에서 엄폐 이동 확률 (%). EnemyData에서 설정"))
    float SeekCoverChance = 70.f;

    // --- Forget ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Forget", meta=(ToolTip="스폰 지점에서 이 거리(cm) 넘게 멀어지면 타겟을 잊음. 0=제한 없음. EnemyData에서 설정"))
    float LeashDistance = 2500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Forget", meta=(ToolTip="타겟을 이 시간(초) 넘게 못 보면 잊음. 0=잊지 않음. EnemyData에서 설정"))
    float ForgetTime = 10.f;

    // --- Reload ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Reload", meta=(ClampMin="0", ClampMax="1", ToolTip="잔탄 비율이 이 값 이하면 재장전 필요 (0=빌 때만). EnemyData에서 설정"))
    float ReloadAmmoRatio = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Reload", meta=(ToolTip="true면 예비 탄약 무한. EnemyData에서 설정"))
    bool bInfiniteReserveAmmo = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Reload", meta=(ToolTip="재장전 몽타주. EnemyData에서 설정"))
    UAnimMontage* ReloadMontage = nullptr;

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

    UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
    float GetAttackCooldown() const { return AttackCooldown; }

    UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
    float GetAttackDamage() const { return AttackDamage; }

    // --- Combat ---

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void FireAtTarget();

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void StopFiring();

    UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
    void AlertEnemy(AActor* Target);

    // 타겟을 잊을 때 경계 해제 — 다시 감지되면 AlertEnemy가 아군 전파까지 새로 수행하게 한다
    UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
    void ResetAlert() { bIsAlerted = false; bCanSeeTarget = false; }

    // --- Behavior data (BT 노드가 EnemyData 값을 읽는 창구) ---

    UFUNCTION(BlueprintPure, Category = "Enemy|Behavior Chance")
    float GetBehaviorChance(EEnemyBehaviorChance Behavior) const;

    UFUNCTION(BlueprintPure, Category = "Enemy|Fire Pattern")
    void GetFirePattern(EEnemyFirePattern Pattern, float& OutBurst, float& OutRest) const;

    UFUNCTION(BlueprintPure, Category = "Enemy|Forget")
    float GetLeashDistance() const { return LeashDistance; }

    UFUNCTION(BlueprintPure, Category = "Enemy|Forget")
    float GetForgetTime() const { return ForgetTime; }

    // --- Weapon state (BT 태스크·서비스가 무기 내부를 직접 만지지 않도록 캐릭터가 창구 역할) ---

    UFUNCTION(BlueprintPure, Category = "Enemy|Weapon")
    AWeaponBase* GetEnemyWeapon() const { return EnemyWeapon; }

    UFUNCTION(BlueprintPure, Category = "Enemy|Reload")
    bool NeedsReload() const;

    UFUNCTION(BlueprintPure, Category = "Enemy|Reload")
    bool IsReloading() const;

    // 재장전 시작. 이미 가득 찼거나 장전 중이면 false
    UFUNCTION(BlueprintCallable, Category = "Enemy|Reload")
    bool ReloadWeapon();

    // --- Movement ---

    UFUNCTION(BlueprintCallable, Category = "Enemy|Movement")
    void SetMoveMode(EEnemyMoveMode NewMode);

    UFUNCTION(BlueprintPure, Category = "Enemy|Movement")
    EEnemyMoveMode GetMoveMode() const { return MoveMode; }

    UFUNCTION(BlueprintPure, Category = "Enemy|Patrol")
    float GetPatrolRadius() const { return PatrolRadius; }

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
