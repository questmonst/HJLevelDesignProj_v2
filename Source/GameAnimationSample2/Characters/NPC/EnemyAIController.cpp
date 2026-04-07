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

        if (Enemy) Enemy->AlertEnemy(Actor);
    }
    else
    {
        // 타겟 위치는 마지막 목격 위치로 유지, 시야만 끊김
        BB->SetValueAsBool(AEnemyCharacter::BBKey_bCanSeeTarget, false);

        if (Enemy) Enemy->StopFiring();
    }
}
