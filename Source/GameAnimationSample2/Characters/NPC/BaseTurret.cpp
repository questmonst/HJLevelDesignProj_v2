// Copyright Epic Games, Inc. All Rights Reserved.
#include "BaseTurret.h"
#include "WeaponBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"

ABaseTurret::ABaseTurret()
{
    PrimaryActorTick.bCanEverTick = true;

    TurretRoot  = CreateDefaultSubobject<USceneComponent>(TEXT("TurretRoot"));
    SetRootComponent(TurretRoot);

    BarrelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot"));
    BarrelPivot->SetupAttachment(TurretRoot);

    // Perception
    SightConfig                            = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius               = AttackRange;
    SightConfig->LoseSightRadius           = AttackRange + 200.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;
    SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals  = false;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    AIPerception->ConfigureSense(*SightConfig);
    AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseTurret::OnTargetPerceptionUpdated);
}

void ABaseTurret::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;

    if (WeaponClass)
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        TurretWeapon = GetWorld()->SpawnActor<AWeaponBase>(
            WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
        if (TurretWeapon)
        {
            TurretWeapon->AttachToComponent(
                BarrelPivot,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                WeaponAttachSocket);
        }
    }
}

void ABaseTurret::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!CurrentTarget) return;

    RotateTowardTarget(DeltaTime);
    TryFire(DeltaTime);
}

void ABaseTurret::RotateTowardTarget(float DeltaTime)
{
    FVector ToTarget = (CurrentTarget->GetActorLocation() - BarrelPivot->GetComponentLocation()).GetSafeNormal();
    FRotator TargetRot  = ToTarget.Rotation();
    FRotator CurrentRot = BarrelPivot->GetComponentRotation();
    FRotator NewRot     = FMath::RInterpConstantTo(CurrentRot, TargetRot, DeltaTime, RotationSpeed);
    BarrelPivot->SetWorldRotation(NewRot);
}

void ABaseTurret::TryFire(float DeltaTime)
{
    FireCooldownRemaining -= DeltaTime;
    if (FireCooldownRemaining > 0.f) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget > AttackRange) return;

    if (TurretWeapon) TurretWeapon->StartFire();
    FireCooldownRemaining = AttackCooldown;
}

float ABaseTurret::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth -= DamageAmount;
    if (CurrentHealth <= 0.f)
    {
        if (TurretWeapon) TurretWeapon->StopFire();
        OnTurretDestroyed();
    }
    return DamageAmount;
}

void ABaseTurret::OnTurretDestroyed_Implementation()
{
    SetActorTickEnabled(false);
    // BP에서 Chaos 파괴/이펙트 처리
}

void ABaseTurret::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        CurrentTarget = Actor;
    }
    else
    {
        if (CurrentTarget == Actor)
        {
            CurrentTarget = nullptr;
            if (TurretWeapon) TurretWeapon->StopFire();
        }
    }
}
