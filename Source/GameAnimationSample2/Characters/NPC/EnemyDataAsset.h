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

	// --- Ragdoll ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(ToolTip="켜면 죽을 때 렉돌로 쓰러지고, 플레이어 화면 밖으로 나가면 사라진다. 끄면 기존대로 DeathEffectDuration 뒤 그냥 제거"))
	bool bRagdollOnDeath = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(ToolTip="사망 몽타주 목록. 이 중 하나를 무작위로 재생. 비우면 즉시 렉돌. 슬롯이 적 ABP에 있어야 재생된다"))
	TArray<UAnimMontage*> DeathMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(ClampMin="0", ClampMax="1", EditCondition="bRagdollOnDeath", ToolTip="사망 몽타주를 어디까지 보여주고 렉돌로 넘어갈지. 0=즉시 렉돌, 0.5=절반 재생 후, 1=끝까지 재생 후. 몽타주가 없으면 무조건 즉시"))
	float RagdollDelayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(ClampMin="0", EditCondition="bRagdollOnDeath", ToolTip="쓰러진 뒤 최소 이 시간(초)은 남아 있는다. 이 시간 전에는 화면 밖이어도 안 사라짐"))
	float CorpseMinTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(ClampMin="0", EditCondition="bRagdollOnDeath", ToolTip="화면 밖으로 나가길 기다리는 최대 시간(초). 계속 쳐다보고 있어도 이 시간이 지나면 제거 — 시체가 무한정 쌓이는 걸 막는 안전장치"))
	float CorpseMaxTime = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(ToolTip="켜면 미카 펀치 넉백에 렉돌로 굴렀다가 일어난다. 끄면 기존대로 밀려남"))
	bool bRagdollOnKnockback = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(ClampMin="0.1", EditCondition="bRagdollOnKnockback", ToolTip="넉백 렉돌 최대 시간(초). 계단을 굴러떨어지는 등 안 멈춰도 이 시간이 되면 일어난다"))
	float KnockbackRagdollMaxTime = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(ClampMin="0", EditCondition="bRagdollOnKnockback", ToolTip="넉백 렉돌 최소 시간(초). 날아가는 도중에 일어나 버리지 않게"))
	float KnockbackRagdollMinTime = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(ClampMin="0", EditCondition="bRagdollOnKnockback", ToolTip="이 속도(cm/s) 아래로 느려지면 착지·정지로 보고 일어난다. 크게 잡으면 구르는 중에 일어나고, 너무 작으면 미세하게 떨릴 때 못 일어난다"))
	float KnockbackSettleSpeed = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(EditCondition="bRagdollOnKnockback", ToolTip="일어날 때 재생할 몽타주. 비우면 즉시 선 자세로 복귀"))
	UAnimMontage* GetUpMontage = nullptr;

	// --- Hit Reaction ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReact", meta=(ToolTip="피격 반응 몽타주 목록. 이 중 하나를 무작위로 재생. 상체 슬롯으로 만들면 이동 중에도 자연스럽게 섞인다"))
	TArray<UAnimMontage*> HitMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReact", meta=(ClampMin="0", ToolTip="피격 몽타주 블렌드 시간(초). 현재 동작에서 부드럽게 섞여 들어간다"))
	float HitMontageBlendTime = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReact", meta=(ClampMin="0", ToolTip="피격 몽타주 최소 재생 간격(초). 연사에 맞으면 매 발 처음부터 다시 재생돼 덜덜 떨리므로 제한"))
	float HitMontageMinInterval = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta=(ToolTip="골반 본 이름. 일어날 때 캡슐을 이 본 아래 바닥으로 옮긴다. 스켈레톤에 맞춰 지정 (UE 마네킹 = pelvis)"))
	FName RagdollPelvisBone = TEXT("pelvis");
};
