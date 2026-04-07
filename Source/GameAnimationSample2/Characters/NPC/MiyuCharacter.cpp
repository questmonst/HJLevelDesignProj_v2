// Copyright Epic Games, Inc. All Rights Reserved.

#include "MiyuCharacter.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AMiyuCharacter::AMiyuCharacter()
{
	// 미유는 아군
	bIsAlly = true;
}

void AMiyuCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 플레이어 레퍼런스 캐시 (LoS 차단 판정에 사용)
	PlayerRef = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

// ────────────────────────────────────────────────
// Target Control
// ────────────────────────────────────────────────

void AMiyuCharacter::SetTarget(AActor* NewTarget)
{
	if (CurrentTarget == NewTarget) return;

	// 기존 타겟이 있으면 먼저 해제
	if (bIsFiring)
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
		bIsFiring = false;
	}

	CurrentTarget = NewTarget;

	if (CurrentTarget)
	{
		OnTargetAcquired(CurrentTarget);

		// FirstFireDelay 후 첫 체크, 이후 FireCooldown 주기 반복
		GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&AMiyuCharacter::TryFire,
			FireCooldown,
			/*bLoop=*/true,
			FirstFireDelay);

		bIsFiring = true;
	}
}

void AMiyuCharacter::ClearTarget()
{
	if (bIsFiring)
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
		bIsFiring = false;
		OnTargetLost();
	}
	CurrentTarget = nullptr;
}

// ────────────────────────────────────────────────
// Query
// ────────────────────────────────────────────────

bool AMiyuCharacter::CanFireAtTarget() const
{
	if (!CurrentTarget || !GetWorld()) return false;

	// 1. 거리 체크
	const float DistSq = FVector::DistSquared(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (DistSq > FMath::Square(SupportFireRange)) return false;

	// 2. LoS + 플레이어 미차단 체크
	//    HasLineOfSightToTarget 은 비-const이므로 const_cast 사용
	return const_cast<AMiyuCharacter*>(this)->HasLineOfSightToTarget(CurrentTarget);
}

bool AMiyuCharacter::HasLineOfSightToTarget(AActor* Target)
{
	if (!Target || !GetWorld()) return false;

	const FVector Start = GetActorLocation();
	const FVector End   = Target->GetActorLocation();

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Target);
	// ※ PlayerRef는 무시 목록에 추가하지 않음.
	//   플레이어가 Miyu와 Target 사이에 있으면 LineTrace가 플레이어에 막혀
	//   bHit = true → return false → 발사 안 함.
	//   플레이어가 비켜주면 → false → 발사 가능.

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

	return !bHit;
}

// ────────────────────────────────────────────────
// Internal
// ────────────────────────────────────────────────

void AMiyuCharacter::TryFire()
{
	// 타겟이 소멸됐거나 유효하지 않으면 자동 해제
	if (!IsValid(CurrentTarget))
	{
		ClearTarget();
		return;
	}

	if (CanFireAtTarget())
	{
		OnSupportFire(CurrentTarget);
	}
}

// ────────────────────────────────────────────────
// Events — Default Implementation (BP에서 오버라이드)
// ────────────────────────────────────────────────

void AMiyuCharacter::OnSupportFire_Implementation(AActor* Target)
{
	// BP에서 오버라이드:
	//   1. 사격 몽타주 재생 (PlayAnimMontage)
	//   2. 발사체 스폰 or 히트스캔 (Target 방향)
}

void AMiyuCharacter::OnTargetAcquired_Implementation(AActor* Target)
{
	// BP에서 오버라이드:
	//   경계 전환 애니메이션, 타겟 방향 회전 시작 등
}

void AMiyuCharacter::OnTargetLost_Implementation()
{
	// BP에서 오버라이드:
	//   대기 애니메이션 복귀, UI 숨기기 등
}
