// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyAIController.h"
#include "EnemyCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

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

    AIPerception->ConfigureSense(*SightConfig);
    AIPerception->ConfigureSense(*HearingConfig);
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
    if (!BB) return;

    AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());

    if (Stimulus.WasSuccessfullySensed())
    {
        BB->SetValueAsObject(AEnemyCharacter::BBKey_TargetActor,   Actor);
        BB->SetValueAsVector(AEnemyCharacter::BBKey_TargetLocation, Actor->GetActorLocation());
        BB->SetValueAsBool  (AEnemyCharacter::BBKey_bCanSeeTarget,  true);
        BB->SetValueAsBool  (AEnemyCharacter::BBKey_bIsAlerted,     true);
        LastTargetInfoTime = GetWorld()->GetTimeSeconds();

        if (Enemy) Enemy->AlertEnemy(Actor);
    }
    else
    {
        // 시야만 끊김. 마지막 목격 위치는 제압 사격(Fire At Target + TargetLocation)의 조준점
        BB->SetValueAsBool(AEnemyCharacter::BBKey_bCanSeeTarget, false);
        if (FAISystem::IsValidLocation(Stimulus.StimulusLocation))
        {
            BB->SetValueAsVector(AEnemyCharacter::BBKey_TargetLocation, Stimulus.StimulusLocation);
        }
        LastTargetInfoTime = GetWorld()->GetTimeSeconds();

        if (Enemy) Enemy->StopFiring();
    }
}

void AEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateForget();
}

void AEnemyAIController::NotifyDamagedBy(AActor* Attacker)
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB || !Attacker) return;

    // 이미 보고 있으면 사격 분기가 처리 중 — 건드리지 않는다
    if (BB->GetValueAsBool(AEnemyCharacter::BBKey_bCanSeeTarget)) return;

    BB->SetValueAsVector(AEnemyCharacter::BBKey_TargetLocation, Attacker->GetActorLocation());
    BB->SetValueAsBool  (AEnemyCharacter::BBKey_bIsAlerted,     true);
    LastTargetInfoTime = GetWorld()->GetTimeSeconds();
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
    if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn()))
    {
        Enemy->StopFiring();
        Enemy->ResetAlert();
    }
}
