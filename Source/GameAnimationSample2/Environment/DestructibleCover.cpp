// Copyright Epic Games, Inc. All Rights Reserved.

#include "DestructibleCover.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

ADestructibleCover::ADestructibleCover()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));
}

void ADestructibleCover::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

float ADestructibleCover::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDestroyed || DamageAmount <= 0.f) return 0.f;

	LastHitLocation = GetActorLocation();
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		LastHitLocation = static_cast<const FPointDamageEvent&>(DamageEvent).HitInfo.ImpactPoint;
	}
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		LastHitLocation = static_cast<const FRadialDamageEvent&>(DamageEvent).Origin;
	}

	const float Applied = FMath::Min(DamageAmount, CurrentHealth);
	CurrentHealth -= Applied;

	if (CurrentHealth <= 0.f)
	{
		bIsDestroyed  = true;
		CurrentHealth = 0.f;
		OnCoverDestroyed();
	}

	return Applied;
}

void ADestructibleCover::DestroyByPunch_Implementation(AActor* PunchInstigator)
{
    if (bIsDestroyed) return;
    // 펀치는 미카 쪽에서 들어오므로 그쪽 면에서 부서져 반대편으로 흩어진다
    LastHitLocation = PunchInstigator ? PunchInstigator->GetActorLocation() : GetActorLocation();
    bIsDestroyed  = true;
    CurrentHealth = 0.f;
    OnCoverDestroyed();
}

void ADestructibleCover::OnCoverDestroyed_Implementation()
{
	// 콜리전 즉시 제거 (총알·캐릭터가 통과하도록)
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetVisibility(false);

	SpawnDebris();

	// DestroyDelay 후 액터 제거
	SetLifeSpan(DestroyDelay);
}

void ADestructibleCover::SpawnDebris()
{
	if (!FractureCollection) return;

	// 파편은 별도 액터로 — 엄폐물 액터는 곧 사라지지만 파편은 DebrisLifeSpan 동안 남는다.
	// 에셋은 컴포넌트가 등록되기 전에 넣어야 하므로 지연 스폰
	const FTransform SpawnTF = MeshComp->GetComponentTransform();
	AGeometryCollectionActor* Debris = GetWorld()->SpawnActorDeferred<AGeometryCollectionActor>(
		AGeometryCollectionActor::StaticClass(), SpawnTF, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Debris) return;

	if (UGeometryCollectionComponent* GC = Debris->GetGeometryCollectionComponent())
	{
		GC->SetRestCollection(FractureCollection);
	}
	Debris->FinishSpawning(SpawnTF);
	if (DebrisLifeSpan > 0.f) Debris->SetLifeSpan(DebrisLifeSpan);

	// 물리 프록시는 스폰 직후 프레임에 올라가므로, 한 틱 뒤에 부수고 흩뿌린다
	// (같은 프레임에 부르면 아직 클러스터가 없어 한 덩어리로 떨어진다)
	TWeakObjectPtr<AGeometryCollectionActor> WeakDebris = Debris;
	const FVector Origin = LastHitLocation;
	const float Impulse = BreakImpulse;
	const float Radius  = BreakRadius;
	GetWorldTimerManager().SetTimerForNextTick([WeakDebris, Origin, Impulse, Radius]()
	{
		if (!WeakDebris.IsValid()) return;
		if (UGeometryCollectionComponent* GC = WeakDebris->GetGeometryCollectionComponent())
		{
			GC->CrumbleActiveClusters();
			if (Impulse > 0.f)
			{
				GC->AddRadialImpulse(Origin, Radius, Impulse, ERadialImpulseFalloff::RIF_Linear, true);
			}
		}
	});
}
