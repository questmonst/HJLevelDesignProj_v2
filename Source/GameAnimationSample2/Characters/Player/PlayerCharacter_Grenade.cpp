// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "GrenadeBase.h"
#include "WeaponBase.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void APlayerCharacter::StartGrenadeThrow()
{
	if (!GrenadeClass) return;
	bIsPreparingThrow = true;
}

void APlayerCharacter::ReleaseGrenadeThrow()
{
	if (!bIsPreparingThrow) return;
	bIsPreparingThrow = false;
	ClearTrajectory();

	if (!GrenadeClass || GrenadeCount <= 0) return;
	GrenadeCount--;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector ThrowStart    = GetMesh()->GetSocketLocation(WeaponAttachSocket);
	FVector ThrowVelocity = ViewRotation.Vector() * GrenadeThrowSpeed;

	FActorSpawnParameters Params;
	Params.Owner      = this;
	Params.Instigator = GetInstigator();

	AGrenadeBase* Grenade = GetWorld()->SpawnActor<AGrenadeBase>(
		GrenadeClass, ThrowStart, ViewRotation, Params);
	if (Grenade)
	{
		Grenade->Launch(ThrowVelocity);
	}
}

void APlayerCharacter::UpdateTrajectory()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector ThrowStart    = ViewLocation + ViewRotation.Vector() * 50.f;
	FVector ThrowVelocity = ViewRotation.Vector() * GrenadeThrowSpeed;

	FPredictProjectilePathParams PredictParams(
		5.f, ThrowStart, ThrowVelocity, 3.f, ECC_WorldDynamic, this);
	PredictParams.bTraceWithCollision = true;
	PredictParams.SimFrequency        = 15.f;

	FPredictProjectilePathResult Result;
	UGameplayStatics::PredictProjectilePath(this, PredictParams, Result);

	TrajectorySpline->ClearSplinePoints(false);
	for (const FPredictProjectilePathPointData& PointData : Result.PathData)
	{
		TrajectorySpline->AddSplinePoint(PointData.Location, ESplineCoordinateSpace::World, false);
	}
	TrajectorySpline->UpdateSpline();

	int32 NumSegments = FMath::Min(TrajectorySpline->GetNumberOfSplinePoints() - 1, MaxTrajectorySegments);

	for (int32 i = 0; i < SplineMeshPool.Num(); ++i)
	{
		USplineMeshComponent* SMC = SplineMeshPool[i];
		if (i < NumSegments)
		{
			if (TrajectoryMesh && SMC->GetStaticMesh() != TrajectoryMesh)
			{
				SMC->SetStaticMesh(TrajectoryMesh);
				if (TrajectoryMaterial) SMC->SetMaterial(0, TrajectoryMaterial);
			}

			FVector StartPos, StartTangent, EndPos, EndTangent;
			TrajectorySpline->GetLocationAndTangentAtSplinePoint(i,     StartPos, StartTangent, ESplineCoordinateSpace::World);
			TrajectorySpline->GetLocationAndTangentAtSplinePoint(i + 1, EndPos,   EndTangent,   ESplineCoordinateSpace::World);

			float SegmentLength = FVector::Dist(StartPos, EndPos);
			StartTangent = StartTangent.GetSafeNormal() * SegmentLength;
			EndTangent   = EndTangent.GetSafeNormal()   * SegmentLength;

			SMC->SetWorldLocation(StartPos);
			SMC->SetWorldRotation(FRotator::ZeroRotator);
			SMC->SetStartAndEnd(FVector::ZeroVector, StartTangent, EndPos - StartPos, EndTangent);
			SMC->SetStartScale(FVector2D(TrajectoryMeshScale, TrajectoryMeshScale));
			SMC->SetEndScale(FVector2D(TrajectoryMeshScale, TrajectoryMeshScale));
			SMC->SetVisibility(true);
		}
		else
		{
			SMC->SetVisibility(false);
		}
	}
}

void APlayerCharacter::UpdateWeaponTrajectory()
{
	if (!CurrentWeapon) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector LaunchStart    = CurrentWeapon->GetMuzzleLocation();
	FVector LaunchVelocity = ViewRotation.Vector() * CurrentWeapon->GetProjectileSpeedForTrajectory();

	FPredictProjectilePathParams PredictParams(
		5.f, LaunchStart, LaunchVelocity, 3.f, ECC_WorldDynamic, this);
	PredictParams.bTraceWithCollision = true;
	PredictParams.SimFrequency        = 15.f;

	FPredictProjectilePathResult Result;
	UGameplayStatics::PredictProjectilePath(this, PredictParams, Result);

	TrajectorySpline->ClearSplinePoints(false);
	for (const FPredictProjectilePathPointData& PointData : Result.PathData)
	{
		TrajectorySpline->AddSplinePoint(PointData.Location, ESplineCoordinateSpace::World, false);
	}
	TrajectorySpline->UpdateSpline();

	int32 NumSegments = FMath::Min(TrajectorySpline->GetNumberOfSplinePoints() - 1, MaxTrajectorySegments);

	for (int32 i = 0; i < SplineMeshPool.Num(); ++i)
	{
		USplineMeshComponent* SMC = SplineMeshPool[i];
		if (i < NumSegments)
		{
			if (TrajectoryMesh && SMC->GetStaticMesh() != TrajectoryMesh)
			{
				SMC->SetStaticMesh(TrajectoryMesh);
				if (TrajectoryMaterial) SMC->SetMaterial(0, TrajectoryMaterial);
			}

			FVector StartPos, StartTangent, EndPos, EndTangent;
			TrajectorySpline->GetLocationAndTangentAtSplinePoint(i,     StartPos, StartTangent, ESplineCoordinateSpace::World);
			TrajectorySpline->GetLocationAndTangentAtSplinePoint(i + 1, EndPos,   EndTangent,   ESplineCoordinateSpace::World);

			float SegmentLength = FVector::Dist(StartPos, EndPos);
			StartTangent = StartTangent.GetSafeNormal() * SegmentLength;
			EndTangent   = EndTangent.GetSafeNormal()   * SegmentLength;

			SMC->SetWorldLocation(StartPos);
			SMC->SetWorldRotation(FRotator::ZeroRotator);
			SMC->SetStartAndEnd(FVector::ZeroVector, StartTangent, EndPos - StartPos, EndTangent);
			SMC->SetStartScale(FVector2D(TrajectoryMeshScale, TrajectoryMeshScale));
			SMC->SetEndScale(FVector2D(TrajectoryMeshScale, TrajectoryMeshScale));
			SMC->SetVisibility(true);
		}
		else
		{
			SMC->SetVisibility(false);
		}
	}
}

void APlayerCharacter::ClearTrajectory()
{
	for (USplineMeshComponent* SMC : SplineMeshPool)
	{
		SMC->SetVisibility(false);
	}
	TrajectorySpline->ClearSplinePoints();
}
