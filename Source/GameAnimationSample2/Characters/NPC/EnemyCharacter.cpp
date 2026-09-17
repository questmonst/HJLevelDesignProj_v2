// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyCharacter.h"
#include "EnemyAIController.h"
#include "EnemyDataAsset.h"
#include "WeaponBase.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"

// Blackboard 키 이름 정의 (AEnemyAIController와 반드시 일치)
const FName AEnemyCharacter::BBKey_TargetActor    = TEXT("TargetActor");
const FName AEnemyCharacter::BBKey_TargetLocation = TEXT("TargetLocation");
const FName AEnemyCharacter::BBKey_bCanSeeTarget  = TEXT("bCanSeeTarget");
const FName AEnemyCharacter::BBKey_bIsAlerted     = TEXT("bIsAlerted");
const FName AEnemyCharacter::BBKey_PatrolOrigin   = TEXT("PatrolOrigin");
const FName AEnemyCharacter::BBKey_bNeedsReload   = TEXT("bNeedsReload");

AEnemyCharacter::AEnemyCharacter()
{
    AIControllerClass = AEnemyAIController::StaticClass();
    AutoPossessAI     = EAutoPossessAI::PlacedInWorldOrSpawned;
    TeamID            = 1;

    // 적은 머리 위 체력바를 기본 표시 (피격 시 나타났다 일정 시간 후 숨김)
    bShowFloatingHealthBar = true;

    // 엄폐 시 앉기 — 내비 에이전트가 앉기를 허용해야 Crouch()가 동작한다
    GetCharacterMovement()->NavAgentProps.bCanCrouch     = true;
    GetCharacterMovement()->bCrouchMaintainsBaseLocation = true;
}

void AEnemyCharacter::ApplyEnemyData()
{
    if (!EnemyData) return;

    MaxHealth            = EnemyData->MaxHealth;
    WalkSpeed            = EnemyData->WalkSpeed;
    RunSpeed             = EnemyData->RunSpeed;
    CrouchWalkSpeed      = EnemyData->CrouchWalkSpeed;
    AttackRange          = EnemyData->AttackRange;
    PreferredCombatRange = EnemyData->PreferredCombatRange;
    AlertRadius          = EnemyData->AlertRadius;
    ReloadAmmoRatio      = EnemyData->ReloadAmmoRatio;
    bInfiniteReserveAmmo = EnemyData->bInfiniteReserveAmmo;
    ReloadMontage        = EnemyData->ReloadMontage;
    PatrolRadius         = EnemyData->PatrolRadius;
    FireBurstDuration    = EnemyData->FireBurstDuration;
    FireRestDuration     = EnemyData->FireRestDuration;
    SuppressBurstDuration = EnemyData->SuppressBurstDuration;
    SuppressRestDuration = EnemyData->SuppressRestDuration;
    StrafeChance         = EnemyData->StrafeChance;
    SuppressChance       = EnemyData->SuppressChance;
    SeekCoverChance      = EnemyData->SeekCoverChance;
    LeashDistance        = EnemyData->LeashDistance;
    ForgetTime           = EnemyData->ForgetTime;
}

void AEnemyCharacter::BeginPlay()
{
    ApplyEnemyData();
    Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchWalkSpeed;
    SetMoveMode(EEnemyMoveMode::Walk);

    if (DefaultWeaponClass)
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        EnemyWeapon = GetWorld()->SpawnActor<AWeaponBase>(
            DefaultWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

        if (EnemyWeapon)
        {
            EnemyWeapon->AttachToComponent(
                GetMesh(),
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                WeaponAttachSocket);
        }
    }
}

// ---------------------------------------------------------------------------

void AEnemyCharacter::FireAtTarget()
{
    if (EnemyWeapon) EnemyWeapon->StartFire();
}

void AEnemyCharacter::StopFiring()
{
    if (EnemyWeapon) EnemyWeapon->StopFire();
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                  AController* EventInstigator, AActor* DamageCauser)
{
    const float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 아군 오사로 경계하지 않도록 적(AEnemyCharacter)이 쏜 피해는 무시
    APawn* Attacker = EventInstigator ? EventInstigator->GetPawn() : nullptr;
    if (Actual > 0.f && !bIsDead && Attacker && !Attacker->IsA<AEnemyCharacter>())
    {
        if (AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController()))
        {
            AICon->NotifyDamagedBy(Attacker);
        }
    }
    return Actual;
}

float AEnemyCharacter::GetBehaviorChance(EEnemyBehaviorChance Behavior) const
{
    switch (Behavior)
    {
    case EEnemyBehaviorChance::Strafe:    return StrafeChance;
    case EEnemyBehaviorChance::Suppress:  return SuppressChance;
    case EEnemyBehaviorChance::SeekCover: return SeekCoverChance;
    }
    return 0.f;
}

void AEnemyCharacter::GetFirePattern(EEnemyFirePattern Pattern, float& OutBurst, float& OutRest) const
{
    const bool bSuppress = Pattern == EEnemyFirePattern::Suppress;
    OutBurst = bSuppress ? SuppressBurstDuration : FireBurstDuration;
    OutRest  = bSuppress ? SuppressRestDuration  : FireRestDuration;
}

bool AEnemyCharacter::NeedsReload() const
{
    if (!EnemyWeapon || EnemyWeapon->IsReloading()) return false;

    const int32 MagSize = EnemyWeapon->GetMagSize();
    if (MagSize <= 0) return false;

    const bool bHasReserve = bInfiniteReserveAmmo || EnemyWeapon->GetReserveAmmo() > 0;
    const float Ratio = static_cast<float>(EnemyWeapon->GetCurrentAmmo()) / MagSize;
    return bHasReserve && Ratio <= ReloadAmmoRatio && EnemyWeapon->GetCurrentAmmo() < MagSize;
}

bool AEnemyCharacter::IsReloading() const
{
    return EnemyWeapon && EnemyWeapon->IsReloading();
}

bool AEnemyCharacter::ReloadWeapon()
{
    if (!EnemyWeapon || EnemyWeapon->IsReloading()) return false;
    if (EnemyWeapon->GetCurrentAmmo() >= EnemyWeapon->GetMagSize()) return false;

    // 무한 탄약: 장전 직전에 예비탄을 탄창 하나 분량으로 채워 AWeaponBase::Reload의 조건을 통과시킨다
    if (bInfiniteReserveAmmo && EnemyWeapon->GetReserveAmmo() < EnemyWeapon->GetMagSize())
    {
        EnemyWeapon->SetReserveAmmo(EnemyWeapon->GetMagSize());
    }

    StopFiring();
    EnemyWeapon->Reload();
    if (!EnemyWeapon->IsReloading()) return false;

    if (ReloadMontage)
    {
        PlayAnimMontage(ReloadMontage);
    }
    return true;
}

void AEnemyCharacter::SetMoveMode(EEnemyMoveMode NewMode)
{
    MoveMode = NewMode;
    UCharacterMovementComponent* Move = GetCharacterMovement();

    switch (NewMode)
    {
    case EEnemyMoveMode::Crouch:
        Crouch();
        break;
    case EEnemyMoveMode::Run:
        UnCrouch();
        Move->MaxWalkSpeed = RunSpeed;
        break;
    default:
        UnCrouch();
        Move->MaxWalkSpeed = WalkSpeed;
        break;
    }
}

void AEnemyCharacter::AlertEnemy(AActor* Target)
{
    if (bIsAlerted) return;
    bIsAlerted = true;
    OnDetectPlayer();

    // 반경 내 아군에게도 알림 (OverlapSphere로 범위 내 액터만 검색)
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        FCollisionShape::MakeSphere(AlertRadius),
        Params);

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AEnemyCharacter* Ally = Cast<AEnemyCharacter>(Overlap.GetActor());
        if (Ally && !Ally->bIsAlerted)
        {
            Ally->AlertEnemy(Target);
        }
    }
}

// ---------------------------------------------------------------------------

void AEnemyCharacter::OnDetectPlayer_Implementation() {}
void AEnemyCharacter::OnAttack_Implementation()       {}
void AEnemyCharacter::OnLoseSight_Implementation()
{
    bCanSeeTarget = false;
    StopFiring();
}
