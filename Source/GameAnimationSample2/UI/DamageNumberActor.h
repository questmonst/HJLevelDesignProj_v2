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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber", meta=(ToolTip="초당 위로 떠오르는 속도(cm/s). 뜨는 연출을 WBP 애니로 처리하면 0으로"))
	float RiseSpeed = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageNumber", meta=(ToolTip="true면 매 프레임 카메라를 바라봄(빌보드)"))
	bool bFaceCamera = true;

	// 수치 세팅 후 호출 — WBP 애니 재생 등 비주얼 처리를 BP에서
	UFUNCTION(BlueprintImplementableEvent, Category = "DamageNumber")
	void OnInitDamage(float Amount);
};
