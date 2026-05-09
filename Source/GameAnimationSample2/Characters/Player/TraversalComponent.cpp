// Copyright Epic Games, Inc. All Rights Reserved.

#include "TraversalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

UTraversalComponent::UTraversalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTraversalComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ACharacter>(GetOwner());
}

// ---------------------------------------------------------------------------
// TickComponent — bCanTraverse 캐시 갱신
// ---------------------------------------------------------------------------

void UTraversalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    FVector Dummy;
    bCanTraverse = bEnabled && !bOnCooldown && DetectTraversal(Dummy);
}

// ---------------------------------------------------------------------------
// TryTraversal — 장애물 감지 후 목표 지점으로 포물선 점프 (공중 가능)
// ---------------------------------------------------------------------------

bool UTraversalComponent::TryTraversal()
{
    if (!bEnabled || bOnCooldown || !OwnerCharacter) return false;

    FVector LandingPos;
    if (!DetectTraversal(LandingPos)) return false;

    // 쿨다운 시작
    bOnCooldown = true;
    GetWorld()->GetTimerManager().SetTimer(
        CooldownTimerHandle,
        [this]() { bOnCooldown = false; },
        JumpCooldown, false);

    // 포물선 궤적 계산
    const FVector CurrentPos = OwnerCharacter->GetActorLocation();
    const FVector Diff       = LandingPos - CurrentPos;
    const float GravityZ     = FMath::Abs(OwnerCharacter->GetCharacterMovement()->GetGravityZ());

    // 최고점 = 목표-현재 Z차이(최소 0) + JumpArcHeight
    const float PeakHeight  = FMath::Max(Diff.Z, 0.f) + JumpArcHeight;
    const float Vz          = FMath::Sqrt(2.f * GravityZ * PeakHeight);
    const float TimeToPeak  = Vz / GravityZ;
    const float TimeFalling = FMath::Sqrt(2.f * FMath::Max(PeakHeight - Diff.Z, 0.f) / GravityZ);
    const float TotalTime   = TimeToPeak + TimeFalling;

    const FVector HorizVel  = FVector(Diff.X, Diff.Y, 0.f) / TotalTime;
    const FVector LaunchVel = HorizVel + FVector(0.f, 0.f, Vz);

    OwnerCharacter->LaunchCharacter(LaunchVel, true, true);
    return true;
}

// ---------------------------------------------------------------------------
// DetectTraversal — 전방 장애물 감지 및 착지 위치 계산
// ---------------------------------------------------------------------------

bool UTraversalComponent::DetectTraversal(FVector& OutLandingPos) const
{
    if (!OwnerCharacter) return false;

    UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent();
    const float HalfHeight     = Capsule->GetScaledCapsuleHalfHeight();
    const float Radius         = Capsule->GetScaledCapsuleRadius();
    const FVector ActorLoc     = OwnerCharacter->GetActorLocation();
    const FVector Forward      = OwnerCharacter->GetActorForwardVector();
    const float FeetZ          = ActorLoc.Z - HalfHeight;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);
    UWorld* World = OwnerCharacter->GetWorld();

    // ------------------------------------------------------------------
    // Step 1: 허리 높이에서 전방 구체 스윕 — 장애물 벽면 감지
    // ------------------------------------------------------------------
    const FVector SweepStart = ActorLoc + FVector(0.f, 0.f, HalfHeight * 0.2f);
    const FVector SweepEnd   = SweepStart + Forward * ForwardTraceDistance;

    FHitResult WallHit;
    if (!World->SweepSingleByChannel(
        WallHit, SweepStart, SweepEnd,
        FQuat::Identity, ECC_WorldStatic,
        FCollisionShape::MakeSphere(Radius * 0.5f), Params))
    {
        return false;
    }

    // ------------------------------------------------------------------
    // Step 2: 장애물 상단 탐색 — 위에서 아래로 라인 트레이스
    // ------------------------------------------------------------------
    const FVector OverPoint = FVector(
        WallHit.ImpactPoint.X + Forward.X * 30.f,
        WallHit.ImpactPoint.Y + Forward.Y * 30.f,
        FeetZ + MaxObstacleHeight + 30.f);

    FHitResult TopHit;
    if (!World->LineTraceSingleByChannel(
        TopHit, OverPoint,
        FVector(OverPoint.X, OverPoint.Y, FeetZ - 10.f),
        ECC_WorldStatic, Params))
    {
        return false;
    }

    const float RelHeight = TopHit.ImpactPoint.Z - FeetZ;
    if (RelHeight < MinObstacleHeight || RelHeight > MaxObstacleHeight) return false;

    // 착지 목표: 장애물 상단 + ForwardOffset + HeightOffset (캡슐 중심 기준)
    OutLandingPos = FVector(
        TopHit.ImpactPoint.X + Forward.X * LandingForwardOffset,
        TopHit.ImpactPoint.Y + Forward.Y * LandingForwardOffset,
        TopHit.ImpactPoint.Z + HalfHeight + LandingHeightOffset);

    return true;
}
