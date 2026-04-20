// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyCharacter.h"
#include "EnemyAIController.h"
#include "WeaponBase.h"

// Blackboard 키 이름 정의 (AEnemyAIController와 반드시 일치)
const FName AEnemyCharacter::BBKey_TargetActor    = TEXT("TargetActor");
const FName AEnemyCharacter::BBKey_TargetLocation = TEXT("TargetLocation");
const FName AEnemyCharacter::BBKey_bCanSeeTarget  = TEXT("bCanSeeTarget");
const FName AEnemyCharacter::BBKey_bIsAlerted     = TEXT("bIsAlerted");
const FName AEnemyCharacter::BBKey_PatrolOrigin   = TEXT("PatrolOrigin");

AEnemyCharacter::AEnemyCharacter()
{
    AIControllerClass = AEnemyAIController::StaticClass();
    AutoPossessAI     = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

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
