// Copyright Epic Games, Inc. All Rights Reserved.

#include "SplineTrackActor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

ASplineTrackActor::ASplineTrackActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComp"));
	SetRootComponent(SplineComp);
}

void ASplineTrackActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildTrack();
}

void ASplineTrackActor::RebuildTrack()
{
	BuildTrack();
}

USplineMeshComponent* ASplineTrackActor::CreateSplineMeshSegment(float StartDist, float EndDist)
{
	USplineMeshComponent* SMC = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass(), NAME_None, RF_Transient);

	SMC->SetStaticMesh(TrackMesh);
	SMC->SetForwardAxis(ForwardAxis, false);
	if (MeshMaterial) SMC->SetMaterial(0, MeshMaterial);

	SMC->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	if (bEnableCollision) SMC->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	SMC->SetMobility(EComponentMobility::Movable);
	AddInstanceComponent(SMC);
	SMC->AttachToComponent(SplineComp, FAttachmentTransformRules::KeepRelativeTransform);
	SMC->RegisterComponent();

	const FVector StartPos     = SplineComp->GetLocationAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local);
	const FVector StartTangent = SplineComp->GetTangentAtDistanceAlongSpline(StartDist,  ESplineCoordinateSpace::Local).GetClampedToMaxSize(MeshSpacing);
	const FVector EndPos       = SplineComp->GetLocationAtDistanceAlongSpline(EndDist,   ESplineCoordinateSpace::Local);
	const FVector EndTangent   = SplineComp->GetTangentAtDistanceAlongSpline(EndDist,    ESplineCoordinateSpace::Local).GetClampedToMaxSize(MeshSpacing);

	SMC->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent, false);

	if (!FMath::IsNearlyZero(RollOffsetDegrees))
	{
		SMC->SetStartRoll(FMath::DegreesToRadians(RollOffsetDegrees), false);
		SMC->SetEndRoll(FMath::DegreesToRadians(RollOffsetDegrees),   false);
	}

	SMC->UpdateMesh();
	return SMC;
}

void ASplineTrackActor::BuildTrack()
{
	ClearTrackMeshes();

	if (!TrackMesh) return;

	const float SplineLength = SplineComp->GetSplineLength();
	if (SplineLength <= 0.f || MeshSpacing <= 0.f) return;

	const int32 NumSegments = FMath::CeilToInt(SplineLength / MeshSpacing);

	for (int32 i = 0; i < NumSegments; ++i)
	{
		const float StartDist  = i * MeshSpacing;
		const float ClampedEnd = FMath::Min(StartDist + MeshSpacing, SplineLength);
		SplineMeshes.Add(CreateSplineMeshSegment(StartDist, ClampedEnd));
	}

	// 루프 연결 세그먼트
	if (bCloseLoop && !SplineComp->IsClosedLoop() && SplineMeshes.Num() > 0)
	{
		SplineMeshes.Add(CreateSplineMeshSegment(NumSegments * MeshSpacing, 0.f));
	}
}

void ASplineTrackActor::ClearTrackMeshes()
{
	// 배열이 아닌 실제 붙어있는 컴포넌트를 직접 탐색해서 제거
	TArray<USplineMeshComponent*> Existing;
	GetComponents<USplineMeshComponent>(Existing);
	for (USplineMeshComponent* SMC : Existing)
	{
		SMC->UnregisterComponent();
		RemoveInstanceComponent(SMC);
		SMC->DestroyComponent();
	}
	SplineMeshes.Reset();
}
