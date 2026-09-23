// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyAIController.h"
#include "EnemyCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AIPerceptionSystem.h"

AEnemyAIController::AEnemyAIController()
{
    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    SightConfig                            = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius               = SightRadius;
    SightConfig->LoseSightRadius           = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDeg;
    SightConfig->SetMaxAge(5.f);
    SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals  = false;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    HearingConfig              = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(3.f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies   = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals  = false;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

    // 피격 감각 — 시야각 밖(뒤·옆)에서 맞아도 즉시 공격자를 인지하게 한다
    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(5.f);

    AIPerception->ConfigureSense(*SightConfig);
    AIPerception->ConfigureSense(*HearingConfig);
    AIPerception->ConfigureSense(*DamageConfig);
    AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

    PrimaryActorTick.bCanEverTick = true;

    AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(InPawn);
    if (!Enemy) return;

    if (UBehaviorTree* BT = Enemy->GetBehaviorTree())
    {
        RunBehaviorTree(BT);

        // 순찰 원점 초기화
        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsVector(AEnemyCharacter::BBKey_PatrolOrigin, InPawn->GetActorLocation());
        }
    }
}

FGenericTeamId AEnemyAIController::GetGenericTeamId() const
{
    if (const IGenericTeamAgentInterface* TeamPawn = Cast<IGenericTeamAgentInterface>(GetPawn()))
    {
        return TeamPawn->GetGenericTeamId();
    }
    return FGenericTeamId(1);
}

void AEnemyAIController::OnUnPossess()
{
    Super::OnUnPossess();
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB || !Actor) return;

    AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
    const TSubclassOf<UAISense> Sense = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
    const bool bSight  = Sense == UAISense_Sight::StaticClass();
    const bool bDamage = Sense == UAISense_Damage::StaticClass();

    // 총성이 실제로 들어오는지 확인용 — 출력 로그에서 [AIPerception] 검색
    UE_LOG(LogTemp, Log, TEXT("[AIPerception] %s ← %s (%s) sensed=%d dist=%.0f"),
        *GetNameSafe(GetPawn()), *GetNameSafe(Actor), *GetNameSafe(Sense.Get()),
        Stimulus.WasSuccessfullySensed() ? 1 : 0,
        GetPawn() ? FVector::Dist(GetPawn()->GetActorLocation(), Stimulus.StimulusLocation) : -1.f);

    if (Stimulus.WasSuccessfullySensed())
    {
        BB->SetValueAsObject(AEnemyCharacter::BBKey_TargetActor,   Actor);
        BB->SetValueAsVector(AEnemyCharacter::BBKey_TargetLocation, Actor->GetActorLocation());
        BB->SetValueAsBool  (AEnemyCharacter::BBKey_bIsAlerted,     true);
        LastTargetInfoTime = GetWorld()->GetTimeSeconds();

        // "볼 수 있음"은 시야 감각이 결정한다. 청각은 경계·위치만 갱신.
        // 피격은 공격자가 시선(LOS) 안에 있으면 즉시 교전 — 포커스가 돌아가면 시야 감각이 이어받는다
        if (bSight)
        {
            BB->SetValueAsBool(AEnemyCharacter::BBKey_bCanSeeTarget, true);
            bDamageEngaged = false;
        }
        else if (bDamage && LineOfSightTo(Actor))
        {
            BB->SetValueAsBool(AEnemyCharacter::BBKey_bCanSeeTarget, true);
            bDamageEngaged   = true;
            DamageEngageTime = GetWorld()->GetTimeSeconds();
        }

        // 감지하면 그 쪽을 쳐다본다 — 총성(청각)만 들었을 때도 돌아보게 한다
        SetFocus(Actor, EAIFocusPriority::Gameplay);
        if (Enemy) Enemy->SetFaceTargetMode(true);

        if (Enemy) Enemy->AlertEnemy(Actor);
    }
    else if (bSight)
    {
        // 시야만 끊김. 마지막 목격 위치는 제압 사격(Fire At Target + TargetLocation)의 조준점
        BB->SetValueAsBool(AEnemyCharacter::BBKey_bCanSeeTarget, false);
        if (FAISystem::IsValidLocation(Stimulus.StimulusLocation))
        {
            BB->SetValueAsVector(AEnemyCharacter::BBKey_TargetLocation, Stimulus.StimulusLocation);
        }
        LastTargetInfoTime = GetWorld()->GetTimeSeconds();
        bDamageEngaged = false;

        if (Enemy) Enemy->StopFiring();
    }
}

void AEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateDamageEngage();
    UpdateForget();
}

void AEnemyAIController::UpdateDamageEngage()
{
    if (!bDamageEngaged || GetWorld()->GetTimeSeconds() - DamageEngageTime < DamageEngageGraceTime) return;
    bDamageEngaged = false;

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB) return;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(AEnemyCharacter::BBKey_TargetActor));
    if (Target && IsSightSensing(Target)) return;   // 시야가 이어받음 — 이후 해제는 시야 감각이 처리

    // 돌아봤는데 못 봤다: 교전 해제, 경계는 유지(TargetLocation 조사·잊기 타이머로 넘어감)
    BB->SetValueAsBool(AEnemyCharacter::BBKey_bCanSeeTarget, false);
    if (Target) BB->SetValueAsVector(AEnemyCharacter::BBKey_TargetLocation, Target->GetActorLocation());
    LastTargetInfoTime = GetWorld()->GetTimeSeconds();
    if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn())) Enemy->StopFiring();
}

bool AEnemyAIController::IsSightSensing(AActor* Actor) const
{
    TArray<AActor*> Seen;
    AIPerception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), Seen);
    return Seen.Contains(Actor);
}

void AEnemyAIController::UpdateForget()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    const AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
    if (!BB || !Enemy || !BB->GetValueAsBool(AEnemyCharacter::BBKey_bIsAlerted)) return;

    // 스폰 지점 이탈 — 보고 있어도 잊는다(추격 한계). 시야를 잃었다 다시 잡으면 재감지된다
    const float Leash = Enemy->GetLeashDistance();
    if (Leash > 0.f)
    {
        const FVector Origin = BB->GetValueAsVector(AEnemyCharacter::BBKey_PatrolOrigin);
        if (FVector::Dist(Enemy->GetActorLocation(), Origin) > Leash)
        {
            ForgetTarget();
            return;
        }
    }

    // 장기간 미발견
    const float ForgetTime = Enemy->GetForgetTime();
    if (ForgetTime > 0.f
        && !BB->GetValueAsBool(AEnemyCharacter::BBKey_bCanSeeTarget)
        && GetWorld()->GetTimeSeconds() - LastTargetInfoTime > ForgetTime)
    {
        ForgetTarget();
    }
}

void AEnemyAIController::ForgetTarget()
{
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->ClearValue(AEnemyCharacter::BBKey_TargetActor);
        BB->ClearValue(AEnemyCharacter::BBKey_TargetLocation);
        BB->SetValueAsBool(AEnemyCharacter::BBKey_bCanSeeTarget, false);
        BB->SetValueAsBool(AEnemyCharacter::BBKey_bIsAlerted,    false);
    }
    ClearFocus(EAIFocusPriority::Gameplay);
    if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn()))
    {
        Enemy->StopFiring();
        Enemy->ResetAlert();
        Enemy->SetFaceTargetMode(false);   // 다시 이동 방향으로 회전
    }
}
