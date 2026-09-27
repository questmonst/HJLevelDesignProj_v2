// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LevelInteraction.generated.h"

class APlayerCharacter;

// 플레이어가 E로 상호작용할 수 있는 것 (스위치, 수동문 …).
// 감지는 각 액터의 영역 오버랩으로 플레이어 후보 목록에 등록하는 방식 — 플레이어는 가장 가까운 것을 고른다.
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class GAMEANIMATIONSAMPLE2_API IInteractable
{
	GENERATED_BODY()

public:
	// 지금 상호작용할 수 있는가 (적색 상태·이미 열린 문이면 false → 안내도 안 뜬다)
	virtual bool CanInteract(const APlayerCharacter* Player) const { return false; }

	// HUD 안내에 들어갈 행동 이름 (예: "스위치 누르기"). 앞의 "E키:"는 HUDData 형식이 붙인다
	virtual FText GetInteractPrompt() const { return FText::GetEmpty(); }

	virtual void Interact(APlayerCharacter* Player) {}
};

// 레벨 신호를 받는 것 — 스위치를 누르거나 스포너의 적이 전멸하면 연결된 대상에게 보낸다.
// 받은 쪽이 신호를 어떻게 쓸지 정한다 (스위치: 청/적 전환, 문: 청/적 전환 또는 열기, 스포너: 스폰 시작).
// 연결은 보내는 쪽 디테일의 "Signal Targets" 배열에 레벨의 액터를 스포이트로 찍어 넣는다 — 레벨 BP 수정 불필요
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class ULevelSignalReceiver : public UInterface
{
	GENERATED_BODY()
};

class GAMEANIMATIONSAMPLE2_API ILevelSignalReceiver
{
	GENERATED_BODY()

public:
	// bOn: 보내는 쪽 설정값 (켜기/끄기). Source: 보낸 액터
	virtual void ReceiveLevelSignal(bool bOn, AActor* Source) {}
};

namespace LevelSignal
{
	// Targets 중 신호를 받을 수 있는 액터에게 모두 보낸다 (받을 수 없는 액터는 무시)
	GAMEANIMATIONSAMPLE2_API void Send(const TArray<TObjectPtr<AActor>>& Targets, bool bOn, AActor* Source);
}
