// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrenadeBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AGrenadeBase::AGrenadeBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(8.f);
	// 프로젝트의 "Projectile" 프로파일은 월드를 Overlap만 해서 바닥을 통과한다.
	// 수류탄은 바닥·벽에 튕기고(Block) 충돌 폭발해야 하므로 명시적으로 설정.
	// 캐릭터(Pawn)는 통과시키고, 적 피해는 폭발 범위 데미지로 처리한다.
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed        = 0.f;
	ProjectileMovement->MaxSpeed            = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale   = 1.f;
	ProjectileMovement->bShouldBounce       = true;
	ProjectileMovement->Bounciness          = 0.3f;

	// ProjectileVFXComponent는 BeginPlay에서 SpawnSystemAttached로 런타임 생성한다.

	// Impact 모드 충돌 폭발 감지를 위해 히트 이벤트 활성화
	CollisionComp->SetNotifyRigidBodyCollision(true);
	CollisionComp->OnComponentHit.AddDynamic(this, &AGrenadeBase::OnCollision);
}

void AGrenadeBase::BeginPlay()
{
	Super::BeginPlay();

	// 데이터 에셋이 할당되면 모든 설정값을 덮어쓴다 (이후 로직이 이 값을 사용).
	if (GrenadeData)
	{
		Damage                      = GrenadeData->Damage;
		ExplosionRadius             = GrenadeData->ExplosionRadius;
		FuseTime                    = GrenadeData->FuseTime;
		Bounciness                  = GrenadeData->Bounciness;
		DetonationMode              = GrenadeData->DetonationMode;
		ArmingDelay                 = GrenadeData->ArmingDelay;
		SpawnVFX                    = GrenadeData->SpawnVFX;
		ProjectileVFX               = GrenadeData->ProjectileVFX;
		ExplosionVFX                = GrenadeData->ExplosionVFX;
		VisualScale                 = GrenadeData->VisualScale;
		ExplosionVFXReferenceRadius = GrenadeData->ExplosionVFXReferenceRadius;
	}

	ProjectileMovement->Bounciness = Bounciness;

	// 생성 VFX (1회). 본체 비주얼은 Niagara가 담당 (Q3-A) — 메시는 자손에서 비워둠.
	if (SpawnVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), SpawnVFX, GetActorLocation(), GetActorRotation(), FVector(VisualScale));
	}

	// 비행 중 본체(투사체) VFX — 빈 컴포넌트 SetAsset 대신 런타임 부착 생성(렌더 보장)
	if (ProjectileVFX)
	{
		ProjectileVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			ProjectileVFX, RootComponent, NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset, false);
		if (ProjectileVFXComponent)
		{
			ProjectileVFXComponent->SetWorldScale3D(FVector(VisualScale));
		}
	}

	// 폭발 방식 분기
	if (DetonationMode == EGrenadeDetonation::Fuse)
	{
		// 투척용: 신관 시간 후 폭발
		GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &AGrenadeBase::Explode, FuseTime, false);
	}
	else
	{
		// 유탄용: 발사 후 ArmingDelay 뒤 장전 → 이후 첫 충돌 시 폭발
		GetWorldTimerManager().SetTimer(ArmingTimerHandle, this, &AGrenadeBase::OnArmed, ArmingDelay, false);
	}
}

void AGrenadeBase::Launch(const FVector& Velocity)
{
	ProjectileMovement->Velocity = Velocity;
}

void AGrenadeBase::OnArmed()
{
	bArmed = true;
}

void AGrenadeBase::OnCollision(UPrimitiveComponent* /*HitComp*/, AActor* OtherActor,
                               UPrimitiveComponent* /*OtherComp*/, FVector /*NormalImpulse*/,
                               const FHitResult& /*Hit*/)
{
	if (DetonationMode != EGrenadeDetonation::Impact) return;	// Fuse 모드는 충돌로 안 터짐 (튕김)
	if (!bArmed) return;										// 장전 전엔 무시 (즉발·자해 방지)
	if (OtherActor == this || OtherActor == GetOwner()) return;	// 자신·소유자 무시
	Explode();
}

void AGrenadeBase::Explode()
{
	if (bExploded) return;	// 중복 폭발 방지 (다중 충돌·신관 중복)
	bExploded = true;

	GetWorldTimerManager().ClearTimer(FuseTimerHandle);
	GetWorldTimerManager().ClearTimer(ArmingTimerHandle);

	TArray<AActor*> Ignored;
	Ignored.Add(this);
	UGameplayStatics::ApplyRadialDamage(
		this, Damage, GetActorLocation(), ExplosionRadius,
		UDamageType::StaticClass(), Ignored, this, GetInstigatorController(), true);

	// 폭발 VFX — 폭발 반경에 맞춰 스케일 (반경/기준반경 비율)
	if (ExplosionVFX)
	{
		const float Scale = (ExplosionVFXReferenceRadius > 0.f)
			? (ExplosionRadius / ExplosionVFXReferenceRadius) : 1.f;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ExplosionVFX, GetActorLocation(), GetActorRotation(), FVector(Scale));
	}

	if (ProjectileVFXComponent)
	{
		ProjectileVFXComponent->Deactivate();
	}

	OnExplode();
}

void AGrenadeBase::OnExplode_Implementation()
{
	Destroy();
}
