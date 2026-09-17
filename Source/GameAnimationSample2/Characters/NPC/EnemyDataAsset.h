// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyDataAsset.generated.h"

class UAnimMontage;

// 적 한 종류(AR·샷건·저격…)의 "행동" 수치 묶음.
//
// 클래스는 하나, 에셋은 적 종류마다 하나(DA_Enemy_AR 등).
// 무기 자체의 수치(연사 속도·대미지·탄창)는 UWeaponDataAsset 몫이라 여기 두지 않는다 — 중복 방지.
// C++ 재빌드 때 BP 기본값이 초기화되는 문제를 피하려고 수치를 에셋에 보관한다 (UMikaDataAsset과 같은 이유).
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API UEnemyDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta=(ToolTip="최대 체력"))
	float MaxHealth = 100.f;

	// --- Movement ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta=(ToolTip="걷기 속도 (cm/s). 순찰·교전 중 기본 이동"))
	float WalkSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta=(ToolTip="달리기 속도 (cm/s). 엄폐물로 이동할 때 등 BT의 Set Move Mode(Run)에서 사용"))
	float RunSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta=(ToolTip="앉아서 이동하는 속도 (cm/s)"))
	float CrouchWalkSpeed = 150.f;

	// --- Combat ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta=(ToolTip="공격 가능 최대 거리 (cm). Fire At Target은 이 거리 밖이면 실패"))
	float AttackRange = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta=(ToolTip="BT가 유지하려는 이상적 교전 거리 (cm)"))
	float PreferredCombatRange = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta=(ToolTip="피격·감지 시 이 반경(cm) 내 아군에게 경계 전파"))
	float AlertRadius = 1500.f;

	// --- Fire Pattern ---
	// BT의 Fire At Target(Use Enemy Data)이 읽는다. 무기 성격(AR·MG·DMR)은 적 종류별 에셋에서 이 값으로 표현

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Pattern", meta=(ClampMin="0.1", ToolTip="일반 사격 유지 시간 (초)"))
	float FireBurstDuration = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Pattern", meta=(ClampMin="0", ToolTip="일반 사격 후 휴식 시간 (초)"))
	float FireRestDuration = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Pattern", meta=(ClampMin="0.1", ToolTip="제압 사격(마지막 발견 위치) 유지 시간 (초)"))
	float SuppressBurstDuration = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Pattern", meta=(ClampMin="0", ToolTip="제압 사격 후 휴식 시간 (초)"))
	float SuppressRestDuration = 3.f;

	// --- Behavior Chance ---
	// BT의 Random Chance(Use Enemy Data)가 읽는 확률(%)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior Chance", meta=(ClampMin="0", ClampMax="100", ToolTip="사격 시 스트레이핑할 확률 (%)"))
	float StrafeChance = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior Chance", meta=(ClampMin="0", ClampMax="100", ToolTip="타겟이 숨었을 때 마지막 발견 위치에 제압 사격할 확률 (%)"))
	float SuppressChance = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior Chance", meta=(ClampMin="0", ClampMax="100", ToolTip="개활지에 있을 때 엄폐물로 이동할 확률 (%)"))
	float SeekCoverChance = 70.f;

	// --- Forget ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Forget", meta=(ClampMin="0", ToolTip="스폰 지점에서 이 거리(cm) 넘게 멀어지면 타겟을 잊고 복귀. 0=제한 없음"))
	float LeashDistance = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Forget", meta=(ClampMin="0", ToolTip="타겟을 이 시간(초) 넘게 못 보면 잊음. 0=잊지 않음"))
	float ForgetTime = 10.f;

	// --- Reload ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reload", meta=(ClampMin="0", ClampMax="1", ToolTip="탄창 잔탄 비율이 이 값 이하면 재장전 필요로 판단 (0=완전히 빌 때만)"))
	float ReloadAmmoRatio = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reload", meta=(ToolTip="true면 예비 탄약이 줄지 않는다. 적이 탄약 부족으로 무력화되는 것 방지"))
	bool bInfiniteReserveAmmo = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reload", meta=(ToolTip="재장전 시 재생할 몽타주. 비우면 애니 없이 무기 ReloadTime만 기다림"))
	UAnimMontage* ReloadMontage = nullptr;

	// --- Patrol ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol", meta=(ToolTip="순찰 반경 (cm). PatrolOrigin 기준 이 범위 내 랜덤 지점"))
	float PatrolRadius = 500.f;
};
