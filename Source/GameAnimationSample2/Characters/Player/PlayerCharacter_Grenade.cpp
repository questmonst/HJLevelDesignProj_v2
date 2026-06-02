// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "GrenadeBase.h"
#include "WeaponBase.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void APlayerCharacter::StartGrenadeThrow()
{
	if (!GrenadeClass || GrenadeCount <= 0) return;
	if (HeldGrenade) return;	// 이미 손에 들고 있음

	bIsPreparingThrow = true;

	// 손에 드는 연출 모드로 스폰. BeginPlay 전에 SetHeldPresentation을 호출해야 하므로 Deferred 사용.
	// → 생성 FX(NS_Bomb_Spawn)가 끝난 뒤에야 본체 VFX가 손에 붙고 던질 수 있게 된다.
	// 폭발 타이머·이동은 Launch 전까지 작동하지 않으므로 들고 있어도 안전하다.
	const FTransform SpawnTF(GetActorRotation(), GetMesh()->GetSocketLocation(WeaponAttachSocket));
	HeldGrenade = GetWorld()->SpawnActorDeferred<AGrenadeBase>(
		GrenadeClass, SpawnTF, this, GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (HeldGrenade)
	{
		HeldGrenade->SetHeldPresentation(true);
		HeldGrenade->FinishSpawning(SpawnTF);
		HeldGrenade->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			WeaponAttachSocket);
	}
}

void APlayerCharacter::ReleaseGrenadeThrow()
{
	if (!bIsPreparingThrow) return;
	bIsPreparingThrow = false;
	ClearTrajectory();

	if (!HeldGrenade) return;

	// 조준 방향으로 던지기
	FVector ThrowDir = GetActorForwardVector();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
		ThrowDir = ViewRotation.Vector();
	}

	GrenadeCount--;
	// 준비(손 부착 완료) 전이면 Launch가 보류했다가 준비되는 순간 자동 발사한다.
	// 분리(DetachFromActor)도 수류탄 DoLaunch가 처리하므로 여기선 호출만.
	HeldGrenade->Launch(ThrowDir * GrenadeThrowSpeed);
	HeldGrenade = nullptr;
}

void APlayerCharacter::UpdateTrajectory()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// 수류탄 투척: 시야 약간 앞에서 투척 속도로
	const FVector Start    = ViewLocation + ViewRotation.Vector() * 50.f;
	const FVector Velocity = ViewRotation.Vector() * GrenadeThrowSpeed;
	RenderTrajectory(Start, Velocity);
}

void APlayerCharacter::UpdateWeaponTrajectory()
{
	if (!CurrentWeapon) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// 유탄 발사기: 총구에서 무기 발사 속도로
	const FVector Start    = CurrentWeapon->GetMuzzleLocation();
	const FVector Velocity = ViewRotation.Vector() * CurrentWeapon->GetProjectileSpeedForTrajectory();
	RenderTrajectory(Start, Velocity);
}

// 공용 궤도 렌더러: 시작점·초기 속도만 받아 예측 경로를 스플라인 메시 풀로 그린다.
// 수류탄(UpdateTrajectory)·유탄(UpdateWeaponTrajectory)이 각자 다른 시작점/속도로 호출 —
// 표현 로직은 공유하되 입력(시작점·속도)만 다르게.
void APlayerCharacter::RenderTrajectory(const FVector& Start, const FVector& Velocity)
{
	FPredictProjectilePathParams PredictParams(
		5.f, Start, Velocity, 3.f, ECC_WorldDynamic, this);
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
