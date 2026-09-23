// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageNumberActor.generated.h"

class UWidgetComponent;

// 피격마다 피격 지점에 하나씩 스폰되는 독립 플로터 액터.
// 적 캐릭터에 위젯을 붙였다 떼는 대신, 스스로 떠오르며 페이드 후 소멸한다.
// (숫자 여러 개가 각자 독립적으로 겹칠 수 있게 하기 위함)
UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API ADamageNumberActor : public AActor
{
	GENERATED_BODY()

public:
	ADamageNumberActor();

	// 스폰 직후 C++이 호출 — 수치 저장 후 BP 훅(OnInitDamage) 실행
	UFUNCTION(BlueprintCallable, Category = "DamageNumber")
	void InitDamage(float Amount);

	UFUNCTION(BlueprintPure, Category = "DamageNumber")
	float GetDamageAmount() const { return DamageAmount; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DamageNumber", meta=(ToolTip="숫자를 표시할 위젯 컴포넌트 (World 스페이스)"))
	UWidgetComponent* WidgetComp;

	UPROPERTY(BlueprintReadOnly, Category = "DamageNumber", meta=(ToolTip="표시할 대미지 수치. WBP에서 텍스트에 바인딩"))
	float DamageAmount = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber", meta=(ToolTip="자동 소멸까지 시간(초)"))
	float LifeSeconds = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber", meta=(ToolTip="초당 위로 떠오르는 속도(cm/s). 포물선(bArcMotion)을 쓰면 무시된다"))
	float RiseSpeed = 60.f;

	// --- 포물선 연출 ---
	// 머리 옆으로 튀어 올랐다가 중력을 받아 떨어진다. 좌우는 카메라 기준이라 화면에서 항상 옆으로 보인다.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber|Arc", meta=(ToolTip="true면 좌우로 튀어 올랐다가 떨어지는 포물선으로 움직인다"))
	bool bArcMotion = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber|Arc", meta=(ClampMin="0", ToolTip="처음 위로 솟는 속도 (cm/s)"))
	float ArcUpSpeed = 260.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber|Arc", meta=(ClampMin="0", ToolTip="좌우로 퍼지는 속도 (cm/s). 방향은 무작위로 왼쪽/오른쪽"))
	float ArcSideSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber|Arc", meta=(ClampMin="0", ClampMax="1", ToolTip="좌우·상승 속도의 무작위 편차 비율. 0.3이면 ±30%"))
	float ArcSpeedVariance = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber|Arc", meta=(ToolTip="낙하 가속도 (cm/s²). 음수가 아래로"))
	float ArcGravity = -800.f;

	FVector ArcVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber", meta=(ToolTip="true면 매 프레임 카메라를 바라봄(빌보드). 화면 공간(bScreenSpace)일 땐 필요 없다"))
	bool bFaceCamera = true;

	// 화면 공간 위젯 — 항상 카메라 정면, 거리와 무관하게 같은 크기, 벽·바닥에 묻히지 않음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber", meta=(ToolTip="true면 화면 공간으로 그린다. 항상 정면·일정 크기이고 지형에 가려지지 않는다"))
	bool bScreenSpace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber", meta=(ClampMin="0", ClampMax="1", ToolTip="수명의 몇 % 지점부터 서서히 사라질지. 0.6이면 60%부터"))
	float FadeStartRatio = 0.55f;

	float SpawnTime = 0.f;

	// 수치 세팅 후 호출 — WBP 애니 재생 등 비주얼 처리를 BP에서
	UFUNCTION(BlueprintImplementableEvent, Category = "DamageNumber")
	void OnInitDamage(float Amount);
};
