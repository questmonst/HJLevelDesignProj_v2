// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelInteraction.h"
#include "EnemySpawner.generated.h"

class AEnemyCharacter;
class ACharacterBase;
class UBehaviorTree;
class UArrowComponent;
class AEnemySpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawnerClearedSignature, AEnemySpawner*, Spawner);

// 적 스포너 — 정해진 수의 적을 간격을 두고 스폰하고, 전부 죽으면 신호를 보낸다.
//
//  시작: 게임 시작 시(bSpawnOnBeginPlay) / 다른 액터의 레벨 신호(체크포인트·스위치·밟는 트리거) / BP에서 StartSpawning
//  끝:   스폰한 적이 모두 죽으면 OnCleared 방송 + ClearedSignalTargets에 신호 (문·스위치를 청색으로 등)
// 스폰 위치는 SpawnPoints(비우면 이 액터의 화살표)를 돌아가며 쓴다.
//
//  플레이어가 죽었다가 다시 시작하면 (UCheckpointSubsystem::OnPlayerRespawned):
//   - 이미 전멸시킨 스포너  → 그대로 (다시 안 나온다)
//   - 진행 중이던 스포너    → 살아 있는 적을 전부 지우고
//       · 현재 체크포인트 이전(또는 동시)에 시작했으면 → 처음부터 다시 스폰
//       · 체크포인트 이후에 시작했으면              → 시작 전 상태로 되돌림 (트리거를 다시 밟으면 시작)
UCLASS(Blueprintable)
class GAMEANIMATIONSAMPLE2_API AEnemySpawner : public AActor, public ILevelSignalReceiver
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartSpawning();

	// 살아 있는 적을 전부 지운다 (시체는 그대로). 상태는 그대로 — 다시 쓰려면 ResetSpawner
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void DespawnAll();

	// 적을 지우고 시작 전 상태로 (다음 StartSpawning에서 처음부터)
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void ResetSpawner();

	UFUNCTION(BlueprintPure, Category = "Spawner")
	bool HasStarted() const { return bStarted; }

	UFUNCTION(BlueprintPure, Category = "Spawner")
	bool IsCleared() const { return bCleared; }

	// 살아 있는 적 수
	UFUNCTION(BlueprintPure, Category = "Spawner")
	int32 GetAliveCount() const { return AliveEnemies.Num(); }

	UPROPERTY(BlueprintAssignable, Category = "Spawner")
	FOnSpawnerClearedSignature OnCleared;

	// ILevelSignalReceiver — 켜기 신호면 스폰 시작, 끄기 신호면 살아 있는 적 정리
	virtual void ReceiveLevelSignal(bool bOn, AActor* Source) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner", meta=(ToolTip="기본 스폰 위치·방향"))
	UArrowComponent* Arrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta=(ToolTip="스폰할 적 BP (예: BP_AREnemy)"))
	TSubclassOf<AEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta=(ToolTip="비우면 적 BP에 지정된 BT를 쓴다. 넣으면 이 BT로 교체 (예: BT_AR_Hold)"))
	UBehaviorTree* BehaviorTreeOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta=(ClampMin="1", ToolTip="총 스폰 수"))
	int32 SpawnCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta=(ClampMin="0", ToolTip="한 마리씩 스폰하는 간격 (초). 0이면 한꺼번에"))
	float SpawnInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta=(ClampMin="0", ToolTip="동시에 살아 있을 수 있는 최대 수. 꽉 차면 누가 죽을 때까지 기다린다. 0이면 제한 없음"))
	int32 MaxAlive = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta=(ToolTip="게임 시작하자마자 스폰"))
	bool bSpawnOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta=(ToolTip="스폰 위치로 쓸 액터들 (Target Point 등). 순서대로 돌아가며 쓴다. 비우면 이 스포너의 화살표"))
	TArray<TObjectPtr<AActor>> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta=(ToolTip="전멸 시 신호를 보낼 액터 (문·스위치·다음 스포너)"))
	TArray<TObjectPtr<AActor>> ClearedSignalTargets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta=(ToolTip="전멸 시 보낼 신호 값. true = 켜기(청색/열기/스폰 시작)"))
	bool bClearedSignalValue = true;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ACharacterBase>> AliveEnemies;

	int32 SpawnedSoFar = 0;
	bool  bStarted = false;
	bool  bCleared = false;
	float StartTime = -1.f;
	FTimerHandle SpawnTimerHandle;

	// 플레이어가 다시 시작했을 때 — 위 규칙대로 정리·재스폰
	void HandlePlayerRespawned();

	void SpawnNext();
	void ScheduleNext();

	UFUNCTION()
	void HandleEnemyDied(ACharacterBase* DeadCharacter);
};
