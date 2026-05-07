// Copyright Epic Games, Inc. All Rights Reserved.

#include "TraversalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"

UTraversalComponent::UTraversalComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTraversalComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ACharacter>(GetOwner());
}

// ---------------------------------------------------------------------------
// TryTraversal
// ---------------------------------------------------------------------------

bool UTraversalComponent::TryTraversal()
{
    if (!bEnabled || bIsTraversing || !OwnerCharacter) return false;

    ETraversalType Type;
    FTransform WarpTarget, Ledge;

    if (!DetectTraversal(Type, WarpTarget, Ledge)) return false;

    bIsTraversing        = true;
    CurrentTraversalType = Type;
    WarpTargetTransform  = WarpTarget;
    LedgeTransform       = Ledge;

    // MotionWarping 타겟 등록
    if (UMotionWarpingComponent* MWC = OwnerCharacter->FindComponentByClass<UMotionWarpingComponent>())
    {
        MWC->AddOrUpdateWarpTargetFromTransform(FName("TraversalLedge"),   Ledge);
        MWC->AddOrUpdateWarpTargetFromTransform(FName("TraversalLanding"), WarpTarget);
    }

    // 이동 잠금 (애니메이션이 루트모션으로 이동을 처리)
    OwnerCharacter->GetCharacterMovement()->DisableMovement();

    OnTraversalStart(Type, WarpTarget, Ledge);
    return true;
}

// ---------------------------------------------------------------------------
// FinishTraversal — AnimNotify 또는 BP에서 호출
// ---------------------------------------------------------------------------

void UTraversalComponent::FinishTraversal()
{
    if (!bIsTraversing || !OwnerCharacter) return;

    bIsTraversing        = false;
    CurrentTraversalType = ETraversalType::None;

    // 착지 위치로 스냅 (MotionWarping이 없거나 루트모션이 끝나지 않은 경우 보정)
    OwnerCharacter->SetActorLocation(
        WarpTargetTransform.GetLocation(), false, nullptr, ETeleportType::TeleportPhysics);

    OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

// ---------------------------------------------------------------------------
// OnTraversalStart_Implementation — BP에서 오버라이드해 몽타주 재생
// ---------------------------------------------------------------------------

void UTraversalComponent::OnTraversalStart_Implementation(
    ETraversalType Type, const FTransform& WarpTarget, const FTransform& Ledge)
{
    // 기본 구현 없음 — BP에서 Type에 따라 몽타주를 선택해 재생하고
    // 몽타주 종료 AnimNotify에서 FinishTraversal()을 호출하면 됨.
}

// ---------------------------------------------------------------------------
// DetectTraversal — 트레이스로 장애물을 감지하고 타입/목표를 계산
// ---------------------------------------------------------------------------

bool UTraversalComponent::DetectTraversal(
    ETraversalType& OutType, FTransform& OutWarpTarget, FTransform& OutLedge) const
{
    if (!OwnerCharacter) return false;

    UCapsuleComponent* Capsule  = OwnerCharacter->GetCapsuleComponent();
    const float HalfHeight      = Capsule->GetScaledCapsuleHalfHeight();
    const float Radius          = Capsule->GetScaledCapsuleRadius();
    const FVector ActorLoc      = OwnerCharacter->GetActorLocation();
    const FVector Forward       = OwnerCharacter->GetActorForwardVector();
    const float FeetZ           = ActorLoc.Z - HalfHeight;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);
    UWorld* World = OwnerCharacter->GetWorld();

    // ------------------------------------------------------------------
    // Step 1: 허리 높이에서 전방 구체 스윕 — 장애물 벽면 감지
    // ------------------------------------------------------------------
    const FVector SweepStart = ActorLoc + FVector(0.f, 0.f, HalfHeight * 0.2f);
    const FVector SweepEnd   = SweepStart + Forward * ForwardTraceDistance;

    FHitResult WallHit;
    const bool bHitWall = World->SweepSingleByChannel(
        WallHit, SweepStart, SweepEnd,
        FQuat::Identity, ECC_WorldStatic,
        FCollisionShape::MakeSphere(Radius * 0.5f), Params);

    if (!bHitWall) return false;

    // ------------------------------------------------------------------
    // Step 2: 장애물 상단 탐색 — 위에서 아래로 라인 트레이스
    //   - 장애물 표면 바로 위 + 앞쪽 30cm 에서 아래로 쏨
    // ------------------------------------------------------------------
    const FVector OverPoint = FVector(
        WallHit.ImpactPoint.X + Forward.X * 30.f,
        WallHit.ImpactPoint.Y + Forward.Y * 30.f,
        FeetZ + ClimbMaxHeight + 30.f);   // 최대 클라임 높이 위에서 시작

    const FVector DownEnd = FVector(OverPoint.X, OverPoint.Y, FeetZ - 10.f);

    FHitResult TopHit;
    const bool bFoundTop = World->LineTraceSingleByChannel(
        TopHit, OverPoint, DownEnd, ECC_WorldStatic, Params);

    if (!bFoundTop) return false;

    const float RelHeight = TopHit.ImpactPoint.Z - FeetZ;

    if (RelHeight < MinObstacleHeight) return false;
    if (RelHeight > ClimbMaxHeight)    return false;

    // ------------------------------------------------------------------
    // Step 3: 착지 위치 공간 확인 — 캡슐이 들어갈 수 있는지
    // ------------------------------------------------------------------
    const FVector LandingPos = FVector(
        TopHit.ImpactPoint.X + Forward.X * LandingForwardOffset,
        TopHit.ImpactPoint.Y + Forward.Y * LandingForwardOffset,
        TopHit.ImpactPoint.Z + HalfHeight + 2.f);

    FHitResult HeadroomHit;
    const bool bBlocked = World->SweepSingleByChannel(
        HeadroomHit,
        LandingPos, LandingPos + FVector(0.f, 0.f, 2.f),
        FQuat::Identity, ECC_WorldStatic,
        FCollisionShape::MakeCapsule(Radius, HalfHeight), Params);

    if (bBlocked) return false;

    // ------------------------------------------------------------------
    // Step 4: 높이에 따른 타입 결정
    // ------------------------------------------------------------------
    if      (RelHeight <= HurdleMaxHeight) OutType = ETraversalType::Hurdle;
    else if (RelHeight <= VaultMaxHeight)  OutType = ETraversalType::Vault;
    else if (RelHeight <= MantleMaxHeight) OutType = ETraversalType::Mantle;
    else                                   OutType = ETraversalType::Climb;

    const FRotator OwnerRot = OwnerCharacter->GetActorRotation();

    // 착지 트랜스폼 (TraversalLanding)
    OutWarpTarget = FTransform(OwnerRot, LandingPos);

    // 가장자리 트랜스폼 (TraversalLedge) — 장애물 모서리 바로 앞
    const FVector LedgePos = FVector(
        TopHit.ImpactPoint.X,
        TopHit.ImpactPoint.Y,
        TopHit.ImpactPoint.Z);
    OutLedge = FTransform(OwnerRot, LedgePos);

    return true;
}
