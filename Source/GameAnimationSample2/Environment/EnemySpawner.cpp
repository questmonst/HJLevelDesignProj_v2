// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemySpawner.h"
#include "Components/ArrowComponent.h"
#include "EnemyCharacter.h"
#include "TimerManager.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(Root);
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	if (bSpawnOnBeginPlay) StartSpawning();
}

void AEnemySpawner::ReceiveLevelSignal(bool bOn, AActor* Source)
{
	if (bOn) StartSpawning();
}

void AEnemySpawner::StartSpawning()
{
	if (bStarted || !EnemyClass) return;
	bStarted = true;
	SpawnNext();
}

void AEnemySpawner::ScheduleNext()
{
	if (SpawnedSoFar >= SpawnCount) return;
	if (SpawnInterval > 0.f)
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnNext, SpawnInterval, false);
	else
		SpawnNext();
}

void AEnemySpawner::SpawnNext()
{
	if (SpawnedSoFar >= SpawnCount) return;
	// 동시 생존 수가 꽉 찼으면 누가 죽을 때(HandleEnemyDied) 다시 부른다
	if (MaxAlive > 0 && AliveEnemies.Num() >= MaxAlive) return;

	const AActor* Point = SpawnPoints.Num() > 0 ? SpawnPoints[SpawnedSoFar % SpawnPoints.Num()].Get() : nullptr;
	const FTransform SpawnTF = Point ? Point->GetActorTransform() : Arrow->GetComponentTransform();

	// BT는 빙의(스폰 중) 때 실행되므로 교체는 스폰을 끝내기 전에 넣는다
	AEnemyCharacter* Enemy = GetWorld()->SpawnActorDeferred<AEnemyCharacter>(
		EnemyClass, SpawnTF, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	++SpawnedSoFar;
	if (Enemy)
	{
		if (BehaviorTreeOverride) Enemy->SetBehaviorTree(BehaviorTreeOverride);
		Enemy->FinishSpawning(SpawnTF);
		AliveEnemies.Add(Enemy);
		Enemy->OnDied.AddDynamic(this, &AEnemySpawner::HandleEnemyDied);
	}

	ScheduleNext();
}

void AEnemySpawner::HandleEnemyDied(ACharacterBase* DeadCharacter)
{
	AliveEnemies.Remove(DeadCharacter);

	// 동시 생존 제한으로 멈춰 있었다면 이어서 스폰
	if (SpawnedSoFar < SpawnCount && !GetWorldTimerManager().IsTimerActive(SpawnTimerHandle))
	{
		ScheduleNext();
		return;
	}

	if (!bCleared && SpawnedSoFar >= SpawnCount && AliveEnemies.Num() == 0)
	{
		bCleared = true;
		LevelSignal::Send(ClearedSignalTargets, bClearedSignalValue, this);
		OnCleared.Broadcast(this);
	}
}
