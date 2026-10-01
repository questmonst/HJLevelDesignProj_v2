// Copyright Epic Games, Inc. All Rights Reserved.

#include "AttackTokenSubsystem.h"
#include "EnemyCharacter.h"
#include "WeaponBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UAttackTokenSubsystem::SetData(const UCombatDirectorData* InData)
{
	Data = InData;
}

const UCombatDirectorData& UAttackTokenSubsystem::GetData() const
{
	return Data ? *Data : *GetDefault<UCombatDirectorData>();
}

int32 UAttackTokenSubsystem::GetMaxTokens() const
{
	return MaxTokensOverride >= 0 ? MaxTokensOverride : GetData().MaxTokens;
}

void UAttackTokenSubsystem::RegisterCandidate(AEnemyCharacter* Enemy)
{
	if (!Enemy) return;
	Candidates.AddUnique(Enemy);
	if (!WaitingSince.Contains(Enemy)) WaitingSince.Add(Enemy, GetWorld()->GetTimeSeconds());
}

void UAttackTokenSubsystem::UnregisterCandidate(AEnemyCharacter* Enemy)
{
	ReleaseToken(Enemy);
	Candidates.Remove(Enemy);
}

void UAttackTokenSubsystem::ReleaseToken(AEnemyCharacter* Enemy)
{
	if (!Enemy || Holders.Remove(Enemy) == 0) return;

	const float Now = GetWorld()->GetTimeSeconds();
	Enemy->SetAttackToken(false);
	LastReleaseTime.Add(Enemy, Now);
	WaitingSince.Add(Enemy, Now);   // 기다린 시간은 반납한 순간부터 다시 센다
	LastAnyReleaseTime = Now;
}

bool UAttackTokenSubsystem::CanHoldToken(const AEnemyCharacter* Enemy)
{
	if (!Enemy || Enemy->IsDead() || Enemy->IsIncapacitated()) return false;

	// 지금 쏠 수 있고(탄 있음·재장전 아님) 타겟을 보고 있어야 한다
	const AWeaponBase* Weapon = Enemy->GetEnemyWeapon();
	if (!Weapon || !Weapon->CanFire()) return false;

	const AAIController* AICon = Cast<AAIController>(Enemy->GetController());
	const UBlackboardComponent* BB = AICon ? AICon->GetBlackboardComponent() : nullptr;
	return BB && BB->GetValueAsBool(AEnemyCharacter::BBKey_bCanSeeTarget);
}

float UAttackTokenSubsystem::ScoreCandidate(const AEnemyCharacter* Enemy, float Now) const
{
	const UCombatDirectorData& D = GetData();
	float Score = FMath::Max(Enemy->GetTokenWeight(), 0.f);

	if (const APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FVector  ViewLoc;
		FRotator ViewRot;
		PC->GetPlayerViewPoint(ViewLoc, ViewRot);

		// 화면 안(카메라 정면 근처)에 있는 적 우선
		const FVector ToEnemy = (Enemy->GetActorLocation() - ViewLoc).GetSafeNormal();
		if (FVector::DotProduct(ViewRot.Vector(), ToEnemy) >= FMath::Cos(FMath::DegreesToRadians(D.OnScreenHalfAngle)))
		{
			Score *= D.OnScreenMultiplier;
		}

		// 가까운 적 우선 (10m 기준, 너무 가까울 때 점수가 폭발하지 않게 하한)
		if (const APawn* PlayerPawn = PC->GetPawn())
		{
			const float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), Enemy->GetActorLocation());
			Score /= FMath::Max(Dist / 1000.f, 0.25f);
		}
	}

	// 오래 못 받은 적 우선
	if (const float* Since = WaitingSince.Find(Enemy))
	{
		Score *= 1.f + FMath::Max(Now - *Since, 0.f) * D.WaitGrowthPerSecond;
	}
	return Score;
}

void UAttackTokenSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return;

	const float Now = World->GetTimeSeconds();
	const UCombatDirectorData& D = GetData();

	// 토큰을 들 수 없게 된 적(사망·넉백·재장전·시야 상실·사격 태스크 종료)은 회수
	for (int32 i = Holders.Num() - 1; i >= 0; --i)
	{
		AEnemyCharacter* Enemy = Holders[i].Get();
		if (!Enemy)
		{
			Holders.RemoveAt(i);
			LastAnyReleaseTime = Now;
		}
		else if (!CanHoldToken(Enemy) || !Candidates.Contains(Enemy))
		{
			ReleaseToken(Enemy);
		}
	}

	Candidates.RemoveAll([](const TWeakObjectPtr<AEnemyCharacter>& E) { return !E.IsValid(); });
	for (auto It = LastReleaseTime.CreateIterator(); It; ++It) { if (!It.Key().IsValid()) It.RemoveCurrent(); }
	for (auto It = WaitingSince.CreateIterator();    It; ++It) { if (!It.Key().IsValid()) It.RemoveCurrent(); }

	// 빈 자리가 있으면 점수가 가장 높은 후보에게
	while (Holders.Num() < GetMaxTokens() && Now - LastAnyReleaseTime >= D.GlobalGap)
	{
		AEnemyCharacter* Best = nullptr;
		float BestScore = 0.f;
		for (const TWeakObjectPtr<AEnemyCharacter>& Weak : Candidates)
		{
			AEnemyCharacter* Enemy = Weak.Get();
			if (!Enemy || Holders.Contains(Enemy) || !CanHoldToken(Enemy)) continue;

			const float* Released = LastReleaseTime.Find(Enemy);
			if (Released && Now - *Released < D.TokenCooldown) continue;

			const float Score = ScoreCandidate(Enemy, Now);
			if (Score > BestScore)
			{
				BestScore = Score;
				Best = Enemy;
			}
		}
		if (!Best) break;

		Holders.Add(Best);
		Best->SetAttackToken(true);
	}
}
