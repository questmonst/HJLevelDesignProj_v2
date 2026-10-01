// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyCharacter.h"
#include "EnemyAIController.h"
#include "EnemyDataAsset.h"
#include "AttackTokenSubsystem.h"
#include "Perception/AISense_Damage.h"
#include "WeaponBase.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

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

    // 레이저는 총구·조준점 기준으로 매 틱 월드 트랜스폼을 직접 넣는다 (부모 회전·스케일 무시)
    LaserComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserComp"));
    LaserComp->SetupAttachment(RootComponent);
    LaserComp->SetUsingAbsoluteLocation(true);
    LaserComp->SetUsingAbsoluteRotation(true);
    LaserComp->SetUsingAbsoluteScale(true);
    LaserComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LaserComp->SetGenerateOverlapEvents(false);
    LaserComp->SetCanEverAffectNavigation(false);
    LaserComp->SetCastShadow(false);
    LaserComp->SetVisibility(false);
}

void AEnemyCharacter::ApplyEnemyData()
{
    if (!EnemyData) return;

    MaxHealth            = EnemyData->MaxHealth;
    MaxBarrier            = EnemyData->MaxBarrier;
    BarrierDamageMultiplier = EnemyData->BarrierDamageMultiplier;
    BarrierRegenDelay     = EnemyData->BarrierRegenDelay;
    BarrierRegenRate      = EnemyData->BarrierRegenRate;
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

    bRagdollOnDeath      = EnemyData->bRagdollOnDeath;
    DeathMontages        = EnemyData->DeathMontages;
    RagdollDelayRate     = EnemyData->RagdollDelayRate;
    CorpseMinTime        = EnemyData->CorpseMinTime;
    CorpseMaxTime        = EnemyData->CorpseMaxTime;
    bRagdollOnKnockback  = EnemyData->bRagdollOnKnockback;
    KnockbackRagdollMaxTime = EnemyData->KnockbackRagdollMaxTime;
    KnockbackRagdollMinTime = EnemyData->KnockbackRagdollMinTime;
    KnockbackSettleSpeed    = EnemyData->KnockbackSettleSpeed;
    HitMontages             = EnemyData->HitMontages;
    HitMontageBlendTime     = EnemyData->HitMontageBlendTime;
    HitMontageMinInterval   = EnemyData->HitMontageMinInterval;
    GetUpMontage         = EnemyData->GetUpMontage;
    RagdollPelvisBone    = EnemyData->RagdollPelvisBone;

    CombatStance         = EnemyData->CombatStance;
    RelaxedLocomotion    = EnemyData->RelaxedLocomotion;
    HipLocomotion        = EnemyData->HipLocomotion;
    IronsightLocomotion  = EnemyData->IronsightLocomotion;
    LookAtHeightOffset   = EnemyData->LookAtHeightOffset;

    LaserComp->SetStaticMesh(EnemyData->LaserMesh);
    LaserLengthScale     = EnemyData->LaserLengthScale;
    LaserThickness       = EnemyData->LaserThickness;
    TokenWeight          = EnemyData->TokenWeight;
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
    LastFireTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    bIsFiringBurst = true;
    if (EnemyWeapon) EnemyWeapon->StartFire();
}

void AEnemyCharacter::StopFiring()
{
    bIsFiringBurst = false;
    if (EnemyWeapon) EnemyWeapon->StopFire();
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateLaser();
}

bool AEnemyCharacter::ShouldShowLaser() const
{
    // 레이저 = "이 적이 곧 맞는 탄을 쏜다". 공격 토큰을 받고 예고하는 동안에만 켠다 (쏘기 시작하면 끈다)
    if (!bTelegraphing) return false;
    if (bIsDead || IsIncapacitated() || bIsFiringBurst || !EnemyWeapon || EnemyWeapon->IsReloading()) return false;

    const AAIController* AICon = Cast<AAIController>(GetController());
    const UBlackboardComponent* BB = AICon ? AICon->GetBlackboardComponent() : nullptr;
    return BB
        && BB->GetValueAsObject(BBKey_TargetActor) != nullptr
        && BB->GetValueAsBool(BBKey_bCanSeeTarget);
}

bool AEnemyCharacter::GetWeaponMissAim(const FVector& From, FVector& OutDirection, AActor*& OutIgnoredTarget) const
{
    if (IsAccurateFire()) return false;

    const AAIController* AICon = Cast<AAIController>(GetController());
    AActor* Target = AICon ? AICon->GetFocusActor() : nullptr;
    if (!Target) return false;

    // 타겟을 향하는 선에 수직인 원 위의 한 점을 겨눈다 — 탄이 옆을 스쳐 지나가게.
    // 발밑(바닥에 박히는 탄)은 피하려고 아래쪽 절반은 위로 뒤집는다
    const UAttackTokenSubsystem* Director = GetWorld()->GetSubsystem<UAttackTokenSubsystem>();
    const UCombatDirectorData& D = Director ? Director->GetData() : *GetDefault<UCombatDirectorData>();

    const FVector TargetLoc = Target->GetActorLocation();
    const FVector ToTarget  = (TargetLoc - From).GetSafeNormal();
    FVector Right = FVector::CrossProduct(FVector::UpVector, ToTarget).GetSafeNormal();
    if (Right.IsNearlyZero()) Right = FVector::RightVector;
    const FVector Up = FVector::CrossProduct(ToTarget, Right);

    const float Angle  = FMath::FRandRange(0.f, 2.f * PI);
    const float Radius = FMath::FRandRange(D.MissOffsetMin, FMath::Max(D.MissOffsetMin, D.MissOffsetMax));
    const FVector MissPoint = TargetLoc
        + Right * FMath::Cos(Angle) * Radius
        + Up    * FMath::Abs(FMath::Sin(Angle)) * Radius;

    OutDirection     = (MissPoint - From).GetSafeNormal();
    OutIgnoredTarget = Target;   // 플레이어가 그쪽으로 움직여도 맞지 않게 통과시킨다
    return true;
}

void AEnemyCharacter::UpdateLaser()
{
    const UStaticMesh* LaserMesh = LaserComp ? LaserComp->GetStaticMesh() : nullptr;
    const bool bShow = LaserMesh && ShouldShowLaser();
    if (LaserComp && LaserComp->IsVisible() != bShow) LaserComp->SetVisibility(bShow);
    if (!bShow) return;

    // 총구에서 상체가 겨누는 점(= 총알이 향하는 점)까지
    const FVector Start = EnemyWeapon->GetMuzzleLocation();
    const FVector Delta = GetLookAtLocation() - Start;
    const float Dist = Delta.Size();
    if (Dist < KINDA_SMALL_NUMBER) return;

    // 메시 X 길이로 나눠 거리만큼 늘린다 (LaserPointerMesh는 1cm)
    const float MeshLength = FMath::Max(LaserMesh->GetBoundingBox().Max.X, KINDA_SMALL_NUMBER);
    LaserComp->SetWorldLocationAndRotation(Start, Delta.Rotation());
    LaserComp->SetWorldScale3D(FVector(Dist * LaserLengthScale / MeshLength, LaserThickness, LaserThickness));
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                  AController* EventInstigator, AActor* DamageCauser)
{
    const float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 아군 오사로 경계하지 않도록 적(AEnemyCharacter)이 쏜 피해는 무시
    APawn* Attacker = EventInstigator ? EventInstigator->GetPawn() : nullptr;
    if (Actual > 0.f && !bIsDead && Attacker && !Attacker->IsA<AEnemyCharacter>())
    {
        // Perception의 Damage 감각으로 보고 — 감지 처리는 컨트롤러의 퍼셉션 핸들러 한 곳에서
        UAISense_Damage::ReportDamageEvent(this, this, Attacker, Actual, Attacker->GetActorLocation(), GetActorLocation());
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

    // 무한 탄약: 예비탄이 이번 장전에 모자랄 때만 모자란 만큼 채운다.
    // 그래서 예비탄은 쏜 만큼 계속 줄어 0에서 멈추고(떨군 총에서 줍는 탄이 줄어든다), 장전은 언제나 된다
    const int32 Needed = EnemyWeapon->GetMagSize() - EnemyWeapon->GetCurrentAmmo();
    if (bInfiniteReserveAmmo && EnemyWeapon->GetReserveAmmo() < Needed)
    {
        EnemyWeapon->SetReserveAmmo(Needed);
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

void AEnemyCharacter::SetFaceTargetMode(bool bFaceTarget)
{
    bUseControllerRotationYaw = bFaceTarget;
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bOrientRotationToMovement = !bFaceTarget;
    }
}

bool AEnemyCharacter::UpdateAimReady(AActor* Target, float AimDelay)
{
    if (AimDelay <= 0.f) return true;

    const float Now = GetWorld()->GetTimeSeconds();
    // 대상이 바뀌었거나 아직 겨눈 적이 없으면 지금부터 겨눈다
    if (AimTarget.Get() != Target || AimReadyTime < 0.f)
    {
        AimTarget    = Target;
        AimReadyTime = Now + AimDelay;
    }
    return Now >= AimReadyTime;
}

void AEnemyCharacter::ClearAimProgress()
{
    AimTarget.Reset();
    AimReadyTime = -1.f;
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

void AEnemyCharacter::EnterRagdoll()
{
    StopFiring();
    Super::EnterRagdoll();
}

void AEnemyCharacter::OnDeath_Implementation()
{
    StopFiring();

    // BT를 멈추지 않으면 죽은 뒤에도 이동·회전·로코모션이 계속 돌아,
    // 사망 몽타주가 묻히고 그냥 서 있는 것처럼 보인다
    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        if (UBrainComponent* Brain = AICon->GetBrainComponent())
        {
            Brain->StopLogic(TEXT("Dead"));
        }
        AICon->StopMovement();
    }
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    // 캐릭터만 사라지면 무기가 공중에 남고 연사 타이머도 계속 돈다.
    // 미카의 무기 버리기와 같은 경로(SetDropped)로 발밑에 내려놔 줍기도 가능하게 한다
    if (EnemyWeapon)
    {
        const FVector DropLocation = GetActorLocation()
            - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight())
            + GetActorForwardVector() * 40.f;

        EnemyWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        EnemyWeapon->SetActorLocation(DropLocation);
        EnemyWeapon->SetActorRotation(FRotator::ZeroRotator);
        EnemyWeapon->SetOwner(nullptr);
        EnemyWeapon->SetDropped(true);
        EnemyWeapon = nullptr;
    }
    Super::OnDeath_Implementation();
}

void AEnemyCharacter::OnDetectPlayer_Implementation() {}
void AEnemyCharacter::OnAttack_Implementation()       {}
void AEnemyCharacter::OnLoseSight_Implementation()
{
    bCanSeeTarget = false;
    StopFiring();
}

// ---------------------------------------------------------------------------
// Locomotion

bool AEnemyCharacter::IsInCombat() const
{
    if (const AAIController* AICon = Cast<AAIController>(GetController()))
    {
        if (const UBlackboardComponent* BB = AICon->GetBlackboardComponent())
        {
            return BB->GetValueAsBool(BBKey_bIsAlerted);
        }
    }
    return bIsAlerted;
}

void AEnemyCharacter::GetLocomotionPriority(const FEnemyLocomotionSet* OutOrder[3]) const
{
    const FEnemyLocomotionSet* Combat = (CombatStance == EEnemyWeaponStance::Hip) ? &HipLocomotion : &IronsightLocomotion;
    const FEnemyLocomotionSet* Other  = (CombatStance == EEnemyWeaponStance::Hip) ? &IronsightLocomotion : &HipLocomotion;

    if (IsInCombat())
    {
        OutOrder[0] = Combat;  OutOrder[1] = Other;  OutOrder[2] = &RelaxedLocomotion;
    }
    else
    {
        OutOrder[0] = &RelaxedLocomotion;  OutOrder[1] = Combat;  OutOrder[2] = Other;
    }
}

UAnimSequenceBase* AEnemyCharacter::GetLocomotionIdle() const
{
    const FEnemyLocomotionSet* Order[3];
    GetLocomotionPriority(Order);
    for (const FEnemyLocomotionSet* Set : Order)
    {
        if (Set->Idle) return Set->Idle;
    }
    return nullptr;
}

UBlendSpace* AEnemyCharacter::GetLocomotionMove() const
{
    const FEnemyLocomotionSet* Order[3];
    GetLocomotionPriority(Order);
    for (const FEnemyLocomotionSet* Set : Order)
    {
        if (Set->Move) return Set->Move;
    }
    return nullptr;
}

UAnimSequenceBase* AEnemyCharacter::GetStandToCrouchAnim() const
{
    const FEnemyLocomotionSet* Order[3];
    GetLocomotionPriority(Order);
    for (const FEnemyLocomotionSet* Set : Order)
    {
        if (Set->StandToCrouch) return Set->StandToCrouch;
    }
    return nullptr;
}

UAnimSequenceBase* AEnemyCharacter::GetCrouchToStandAnim() const
{
    const FEnemyLocomotionSet* Order[3];
    GetLocomotionPriority(Order);
    for (const FEnemyLocomotionSet* Set : Order)
    {
        if (Set->CrouchToStand) return Set->CrouchToStand;
    }
    return nullptr;
}

FVector AEnemyCharacter::GetLookAtLocation() const
{
    // 사격은 컨트롤러 포커스(액터면 그 위치)를 향하므로 상체도 같은 점을 본다
    if (const AAIController* AICon = Cast<AAIController>(GetController()))
    {
        if (const AActor* Focus = AICon->GetFocusActor())
        {
            return Focus->GetActorLocation() + FVector(0.f, 0.f, LookAtHeightOffset);
        }
        const FVector FocalPoint = AICon->GetFocalPoint();
        if (FAISystem::IsValidLocation(FocalPoint))
        {
            return FocalPoint + FVector(0.f, 0.f, LookAtHeightOffset);
        }
    }
    return GetPawnViewLocation() + GetActorForwardVector() * 1000.f;
}
