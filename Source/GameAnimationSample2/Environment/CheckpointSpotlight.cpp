// Copyright Epic Games, Inc. All Rights Reserved.

#include "CheckpointSpotlight.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "LevelInteraction.h"
#include "PlayerCharacter.h"

static const FName CheckpointColorParam(TEXT("Color"));
static const FName CheckpointIntensityParam(TEXT("Intensity"));

ACheckpointSpotlight::ACheckpointSpotlight()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;   // 켜짐↔꺼짐이 바뀌는 동안만

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	ActivateZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ActivateZone"));
	ActivateZone->SetupAttachment(Root);
	ActivateZone->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	ActivateZone->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	ActivateZone->SetCollisionProfileName(TEXT("Trigger"));
	ActivateZone->SetCanEverAffectNavigation(false);

	// 리스폰 지점 6m 위에서 바로 아래로
	Spotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spotlight"));
	Spotlight->SetupAttachment(Root);
	Spotlight->SetRelativeLocation(FVector(0.f, 0.f, 600.f));
	Spotlight->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	Spotlight->SetInnerConeAngle(12.f);
	Spotlight->SetOuterConeAngle(20.f);
	Spotlight->SetAttenuationRadius(1200.f);

	// 빛줄기·바닥 링은 엔진 기본 도형 (원뿔: 꼭짓점 위, 높이·바닥 지름 100 / 평면: 100×100)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));

	auto SetupVisual = [](UStaticMeshComponent* Comp)
	{
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Comp->SetCanEverAffectNavigation(false);
		Comp->SetCastShadow(false);
	};

	BeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamMesh"));
	BeamMesh->SetupAttachment(Root);
	if (ConeMesh.Succeeded()) BeamMesh->SetStaticMesh(ConeMesh.Object);
	SetupVisual(BeamMesh);

	FloorRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorRing"));
	FloorRing->SetupAttachment(Root);
	if (PlaneMesh.Succeeded()) FloorRing->SetStaticMesh(PlaneMesh.Object);
	SetupVisual(FloorRing);

#if WITH_EDITORONLY_DATA
	RespawnArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("RespawnArrow"));
	if (RespawnArrow)
	{
		RespawnArrow->SetupAttachment(Root);
		RespawnArrow->ArrowColor = FColor::Yellow;
		RespawnArrow->ArrowSize  = 2.f;
		RespawnArrow->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	}
#endif
}

const UCheckpointData& ACheckpointSpotlight::GetData() const
{
	return CheckpointData ? *CheckpointData : *GetDefault<UCheckpointData>();
}

void ACheckpointSpotlight::FitVisuals()
{
	// 원뿔(높이 100, 바닥 반지름 50, 원점 가운데)을 조명 위치~바닥에 딱 맞게 늘린다
	const float Height    = FMath::Max(Spotlight->GetRelativeLocation().Z, 1.f);
	const float FloorRad  = Height * FMath::Tan(FMath::DegreesToRadians(Spotlight->OuterConeAngle));
	BeamMesh->SetRelativeLocation(FVector(0.f, 0.f, Height * 0.5f));
	BeamMesh->SetRelativeRotation(FRotator::ZeroRotator);
	BeamMesh->SetRelativeScale3D(FVector(FloorRad / 50.f, FloorRad / 50.f, Height / 100.f));

	// 바닥 링: 100×100 평면, 바닥에 묻히지 않게 살짝 띄운다
	const float Ring = GetData().RingRadius;
	FloorRing->SetRelativeLocation(FVector(0.f, 0.f, 2.f));
	FloorRing->SetRelativeScale3D(FVector(Ring / 50.f, Ring / 50.f, 1.f));
}

void ACheckpointSpotlight::CreateMaterials()
{
	const UCheckpointData& D = GetData();

	BeamMID = D.BeamMaterial ? BeamMesh->CreateDynamicMaterialInstance(0, D.BeamMaterial) : nullptr;
	BeamMesh->SetVisibility(BeamMID != nullptr);

	RingMID = D.RingMaterial ? FloorRing->CreateDynamicMaterialInstance(0, D.RingMaterial) : nullptr;
	FloorRing->SetVisibility(RingMID != nullptr);
}

void ACheckpointSpotlight::ApplyVisuals()
{
	const UCheckpointData& D = GetData();
	const float A = ActiveAlpha;
	const FLinearColor Color = FLinearColor::LerpUsingHSV(D.InactiveColor, D.ActiveColor, A);

	const float LightIntensity = FMath::Lerp(D.InactiveIntensity, D.ActiveIntensity, A);
	Spotlight->SetLightColor(Color);
	Spotlight->SetIntensity(LightIntensity);
	Spotlight->SetVisibility(LightIntensity > 0.f);

	if (BeamMID)
	{
		BeamMID->SetVectorParameterValue(CheckpointColorParam, Color);
		BeamMID->SetScalarParameterValue(CheckpointIntensityParam, FMath::Lerp(D.InactiveBeamIntensity, D.ActiveBeamIntensity, A));
	}
	if (RingMID)
	{
		RingMID->SetVectorParameterValue(CheckpointColorParam, Color);
		RingMID->SetScalarParameterValue(CheckpointIntensityParam, FMath::Lerp(D.InactiveRingIntensity, D.ActiveRingIntensity, A));
	}
}

void ACheckpointSpotlight::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	// 에디터에서 배치할 때도 꺼진 상태 모습이 보이게
	FitVisuals();
	CreateMaterials();
	ActiveAlpha = 0.f;
	ApplyVisuals();
}

void ACheckpointSpotlight::BeginPlay()
{
	Super::BeginPlay();
	FitVisuals();
	CreateMaterials();
	ActiveAlpha = bIsActive ? 1.f : 0.f;
	ApplyVisuals();
	ActivateZone->OnComponentBeginOverlap.AddDynamic(this, &ACheckpointSpotlight::HandleZoneBegin);
}

void ACheckpointSpotlight::HandleZoneBegin(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!Cast<APlayerCharacter>(Other)) return;
	if (UCheckpointSubsystem* Checkpoints = GetWorld()->GetSubsystem<UCheckpointSubsystem>())
	{
		Checkpoints->TryActivate(this);
	}
}

void ACheckpointSpotlight::SetActiveCheckpoint(bool bActive)
{
	if (bIsActive == bActive) return;
	bIsActive = bActive;

	if (bActive)
	{
		if (GetData().ActivateSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, GetData().ActivateSound, GetActorLocation());
		}
		// 처음 켜질 때 한 번 — 이 구간의 스포너를 깨우는 등
		if (!bSignalSent)
		{
			bSignalSent = true;
			LevelSignal::Send(SignalTargets, true, this);
		}
	}

	if (GetData().BlendTime <= 0.f)
	{
		ActiveAlpha = bIsActive ? 1.f : 0.f;
		ApplyVisuals();
		return;
	}
	SetActorTickEnabled(true);
}

void ACheckpointSpotlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float Target = bIsActive ? 1.f : 0.f;
	ActiveAlpha = FMath::FInterpConstantTo(ActiveAlpha, Target, DeltaTime, 1.f / FMath::Max(GetData().BlendTime, KINDA_SMALL_NUMBER));
	ApplyVisuals();

	if (FMath::IsNearlyEqual(ActiveAlpha, Target)) SetActorTickEnabled(false);
}

// ---------------------------------------------------------------------------

bool UCheckpointSubsystem::FindRespawnPoint(const ACharacter* Character, AController* Controller, FVector& OutLocation, FRotator& OutRotation) const
{
	if (const ACheckpointSpotlight* Checkpoint = Current.Get())
	{
		const FTransform TF = Checkpoint->GetRespawnTransform();
		const float HalfHeight = Character ? Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.f;
		OutLocation = TF.GetLocation() + FVector(0.f, 0.f, HalfHeight);
		OutRotation = FRotator(0.f, TF.Rotator().Yaw, 0.f);
		return true;
	}

	if (AGameModeBase* GM = UGameplayStatics::GetGameMode(this))
	{
		if (const AActor* Start = GM->FindPlayerStart(Controller))
		{
			OutLocation = Start->GetActorLocation();
			OutRotation = FRotator(0.f, Start->GetActorRotation().Yaw, 0.f);
			return true;
		}
	}
	return false;
}

void UCheckpointSubsystem::TryActivate(ACheckpointSpotlight* Checkpoint)
{
	if (!Checkpoint) return;
	ACheckpointSpotlight* Prev = Current.Get();
	if (Prev == Checkpoint) return;

	// 앞으로만: 이미 더 뒤 번호를 밟았으면 앞 번호로 되돌아가도 바꾸지 않는다
	if (Prev && Checkpoint->bOnlyForward && Checkpoint->GetCheckpointOrder() < Prev->GetCheckpointOrder()) return;

	if (Prev) Prev->SetActiveCheckpoint(false);
	Current        = Checkpoint;
	CheckpointTime = GetWorld()->GetTimeSeconds();
	Checkpoint->SetActiveCheckpoint(true);
}
