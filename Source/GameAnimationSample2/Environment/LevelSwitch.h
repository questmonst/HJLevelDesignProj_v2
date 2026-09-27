// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LevelInteractableBase.h"
#include "LevelSwitch.generated.h"

class USoundBase;
class ALevelSwitch;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwitchUsedSignature, ALevelSwitch*, Switch);

// 물리 스위치 — 청색일 때 E로 누르면 연결된 대상(SignalTargets)에 신호를 보내고 OnSwitchUsed를 방송한다.
//
// 연결 방법 두 가지:
//  1) 디테일 › Signal Targets에 문·스포너·다른 스위치를 스포이트로 찍기 (레벨 BP 수정 없음)
//  2) 특별한 연출은 레벨 BP에서 OnSwitchUsed에 바인딩
// 청/적 전환은 SetInteractable 또는 다른 액터가 보내는 레벨 신호로.
UCLASS(Blueprintable)
class GAMEANIMATIONSAMPLE2_API ALevelSwitch : public ALevelInteractableBase
{
	GENERATED_BODY()

public:
	ALevelSwitch();

	// IInteractable
	virtual bool CanInteract(const APlayerCharacter* Player) const override { return bInteractable; }
	virtual FText GetInteractPrompt() const override { return PromptText; }
	virtual void Interact(APlayerCharacter* Player) override;

	UPROPERTY(BlueprintAssignable, Category = "Switch")
	FOnSwitchUsedSignature OnSwitchUsed;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switch", meta=(ToolTip="스위치 본체 메시"))
	UStaticMeshComponent* SwitchMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switch", meta=(ToolTip="HUD 안내 문구 (앞의 'E키:'는 HUDData 형식이 붙인다)"))
	FText PromptText = FText::FromString(TEXT("스위치 누르기"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switch", meta=(ToolTip="눌렀을 때 신호를 보낼 액터 (문·스포너·다른 스위치). 레벨에서 스포이트로 지정"))
	TArray<TObjectPtr<AActor>> SignalTargets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switch", meta=(ToolTip="보낼 신호 값. true = 켜기(청색/열기/스폰 시작), false = 끄기"))
	bool bSignalValue = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switch", meta=(ToolTip="누른 뒤 적색(사용 불가)으로 바뀐다"))
	bool bLockAfterUse = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switch", meta=(ClampMin="0", EditCondition="bLockAfterUse", ToolTip="잠긴 뒤 다시 청색이 되기까지 시간(초). 0이면 신호를 받기 전까지 계속 잠김"))
	float ReuseCooldown = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switch", meta=(ToolTip="눌렀을 때 소리"))
	USoundBase* UseSound = nullptr;

	FTimerHandle ReuseTimerHandle;
	void Unlock() { SetInteractable(true); }
};
