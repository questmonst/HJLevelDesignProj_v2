// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "EnemyCharacter.h"
#include "BTTask_FireAtTarget.generated.h"

// 적이 블랙보드 타겟(액터 또는 위치)을 향해 "사격 → 휴식"을 한 번 수행하는 BT 태스크.
//
// 사격 자체는 AEnemyCharacter::FireAtTarget()이 이미 하고 있으므로, 이 태스크는
// "언제 시작해서 언제 멈추는가"만 책임진다. BP 태스크 대신 C++로 둔 이유는
// BT 노드 목록에 바로 떠서 BP 배선(Cast 노드 등)이 필요 없기 때문.
//
// - 사격/휴식 시간: 기본은 EnemyData의 패턴(일반/제압)을 읽는다 — 같은 BT로 AR·MG 성향 차이를 낸다
// - 연사 속도: 무기(FireRate)가 결정. 연사 무기는 무기 타이머, 단발·점사는 준비될 때마다 재발사
// - Vector 키(TargetLocation)면 제압 사격: 시야 확인 없이 마지막 발견 위치로 쏜다
// - 탄창이 비면 실패 + bNeedsReload를 즉시 세워 재장전 분기로 넘긴다
//
// 조준은 하지 않는다 — 무기가 폰의 시점 방향으로 쏘므로, 타겟을 바라보게 하는 것은
// Set default focus 서비스(액터·위치 키 모두 지원)의 몫이다.
UCLASS()
class GAMEANIMATIONSAMPLE2_API UBTTask_FireAtTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FireAtTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;   // 키 타입(Object/Vector) 확정
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Fire", meta=(ToolTip="사격 대상 키. Object(TargetActor)=일반 사격, Vector(TargetLocation)=제압 사격"))
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, Category = "Fire", meta=(ToolTip="true면 사격/휴식 시간을 적의 EnemyData에서 읽는다"))
	bool bUseEnemyData = true;

	UPROPERTY(EditAnywhere, Category = "Fire", meta=(EditCondition="bUseEnemyData", ToolTip="EnemyData에서 읽을 패턴 (일반/제압)"))
	EEnemyFirePattern Pattern = EEnemyFirePattern::Attack;

	UPROPERTY(EditAnywhere, Category = "Fire", meta=(EditCondition="!bUseEnemyData", ClampMin="0.1", ToolTip="사격 유지 시간(초). EnemyData를 안 쓸 때"))
	float FireDuration = 3.f;

	UPROPERTY(EditAnywhere, Category = "Fire", meta=(EditCondition="!bUseEnemyData", ClampMin="0", ToolTip="사격 후 휴식 시간(초). EnemyData를 안 쓸 때"))
	float RestDuration = 2.f;

	UPROPERTY(EditAnywhere, Category = "Fire", meta=(ClampMin="0", ToolTip="사격·휴식 시간에 더해지는 랜덤 편차(±초). 여러 적이 동시에 멈추는 어색함 방지"))
	float DurationDeviation = 0.f;

	UPROPERTY(EditAnywhere, Category = "Fire", meta=(ToolTip="사거리(AttackRange) 밖이면 실패시킨다. false면 쏘지 않고 성공 처리"))
	bool bFailWhenOutOfRange = true;

	UPROPERTY(EditAnywhere, Category = "Fire", meta=(ToolTip="true면 타겟 액터가 시야에 있을 때만 쏜다. Vector 키(제압 사격)에는 적용 안 됨"))
	bool bRequireLineOfSight = true;

	// --- 교전 시작 조준 ---
	// 플레이어를 막 발견했을 때 바로 쏘면 반응할 틈이 없다. 타겟을 바라본 채 잠깐 겨눈 뒤 쏜다.

	UPROPERTY(EditAnywhere, Category = "Fire|Aim", meta=(ClampMin="0", ToolTip="교전을 새로 시작할 때 타겟을 겨누고 기다리는 시간(초). 0이면 즉시 사격"))
	float AimDelay = 1.f;

	UPROPERTY(EditAnywhere, Category = "Fire|Aim", meta=(ClampMin="0", ToolTip="마지막 사격 후 이 시간이 지났으면 '새 교전'으로 보고 AimDelay를 적용한다. 연속 사격 중에는 기다리지 않는다"))
	float ReengageGap = 3.f;

private:
	void Finish(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type Result) const;
};

// 노드 인스턴스 대신 메모리 구조체 사용 (같은 BT를 쓰는 적끼리 상태 공유 방지)
struct FBTFireAtTargetMemory
{
	float AimTime = 0.f;        // 남은 조준 대기 시간
	float RemainingTime = 0.f;
	float RestTime = 0.f;
	bool bResting = false;
	bool bFiring = false;       // 실제로 사격을 시작했는지
};
