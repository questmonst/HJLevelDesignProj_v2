// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineMeshComponent.h"
#include "SplineTrackActor.generated.h"

class USplineComponent;
class UStaticMesh;
class UMaterialInterface;

/**
 * 스플라인 경로를 따라 메시를 배치하는 액터.
 * 롤러코스터 레일 등 경로형 구조물 제작용.
 * OnConstruction에서 재빌드되므로 에디터에서 스플라인 편집 시 실시간 반영.
 */
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ASplineTrackActor : public AActor
{
	GENERATED_BODY()

public:
	ASplineTrackActor();

protected:
	// ─── 컴포넌트 ───────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline Track")
	USplineComponent* SplineComp;

	// ─── 메시 설정 ──────────────────────────────────────────────
	/** 레일에 반복 배치할 스태틱 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Track|Mesh")
	UStaticMesh* TrackMesh;

	/** 메시 하나의 배치 간격 (cm). 메시 길이에 맞게 조정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Track|Mesh", meta = (ClampMin = "1.0"))
	float MeshSpacing = 200.f;

	/** SplineMesh 변형 기준축 — 메시의 길이 방향에 맞게 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Track|Mesh")
	TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis = ESplineMeshAxis::X;

	/** 메시 롤 오프셋 (도). 레일 방향 보정용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Track|Mesh", meta = (ClampMin = "-180.0", ClampMax = "180.0"))
	float RollOffsetDegrees = 0.f;

	/** 머티리얼 오버라이드 (None이면 메시 기본 머티리얼 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Track|Mesh")
	UMaterialInterface* MeshMaterial;

	/** 콜리전 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Track|Mesh")
	bool bEnableCollision = true;

	/** 스플라인 끝점에서 시작점으로 연결하는 루프 메시 생성 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Track|Mesh")
	bool bCloseLoop = false;

	// ─── 빌드 ───────────────────────────────────────────────────
	virtual void OnConstruction(const FTransform& Transform) override;

	/** 에디터/런타임에서 수동으로 트랙 재빌드 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spline Track")
	void RebuildTrack();

private:
	void BuildTrack();
	void ClearTrackMeshes();
	USplineMeshComponent* CreateSplineMeshSegment(float StartDist, float EndDist);

	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshes;
};
