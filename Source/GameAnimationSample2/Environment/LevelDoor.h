// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LevelInteractableBase.h"
#include "LevelDoor.generated.h"

class USoundBase;
class ALevelDoor;

UENUM(BlueprintType)
enum class EDoorOpenMode : uint8
{
	Auto     UMETA(DisplayName = "자동문 (다가가면 열림)"),
	Interact UMETA(DisplayName = "수동문 (E로 열기)"),
};

UENUM(BlueprintType)
enum class EDoorSignalAction : uint8
{
	SetInteractable UMETA(DisplayName = "청/적 전환"),
	Open            UMETA(DisplayName = "열기/닫기"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDoorStateSignature, ALevelDoor*, Door, bool, bOpened);

// 양쪽으로 미끄러져 열리는 문.
//  - 자동문: 청색이면 영역에 들어갈 때 열리고 나가면 닫힌다. 적색이면 반응 없음
//  - 수동문: 청색이면 영역 안에서 HUD 안내 → E로 연다. 나가면(옵션) 닫힌다
// 문짝은 DoorLeft(-Y 방향) / DoorRight(+Y 방향). 한쪽만 쓰려면 다른 쪽 메시를 비우면 된다.
UCLASS(Blueprintable)
class GAMEANIMATIONSAMPLE2_API ALevelDoor : public ALevelInteractableBase
{
	GENERATED_BODY()

public:
	ALevelDoor();

	virtual void Tick(float DeltaTime) override;

	// IInteractable (수동문만)
	virtual bool CanInteract(const APlayerCharacter* Player) const override;
	virtual FText GetInteractPrompt() const override { return PromptText; }
	virtual void Interact(APlayerCharacter* Player) override;

	// ILevelSignalReceiver
	virtual void ReceiveLevelSignal(bool bOn, AActor* Source) override;

	UFUNCTION(BlueprintCallable, Category = "Door")
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void CloseDoor();

	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsOpen() const { return bWantsOpen; }

	UPROPERTY(BlueprintAssignable, Category = "Door")
	FOnDoorStateSignature OnDoorStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void OnPlayerEnterZone(APlayerCharacter* Player) override;
	virtual void OnPlayerLeaveZone(APlayerCharacter* Player) override;
	virtual void OnInteractableStateChanged() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UStaticMeshComponent* DoorLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UStaticMeshComponent* DoorRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta=(ToolTip="자동문 / 수동문(E)"))
	EDoorOpenMode OpenMode = EDoorOpenMode::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta=(EditCondition="OpenMode==EDoorOpenMode::Interact", ToolTip="HUD 안내 문구"))
	FText PromptText = FText::FromString(TEXT("문 열기"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta=(ToolTip="문짝이 옆으로 밀려나는 거리 (cm)"))
	float OpenDistance = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta=(ClampMin="0.05", ToolTip="완전히 열리거나 닫히는 데 걸리는 시간 (초)"))
	float OpenTime = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta=(ToolTip="플레이어가 영역을 나가면 닫는다 (수동문 옵션. 자동문은 항상 닫힌다)"))
	bool bCloseWhenPlayerLeaves = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta=(ClampMin="0", ToolTip="영역을 나간 뒤 닫힐 때까지 대기 (초)"))
	float CloseDelay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta=(ToolTip="적색으로 바뀌면 열려 있던 문을 닫는다"))
	bool bCloseWhenLocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta=(ToolTip="레벨 신호를 받았을 때: 청/적 전환 또는 바로 열기/닫기"))
	EDoorSignalAction SignalAction = EDoorSignalAction::SetInteractable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	USoundBase* OpenSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	USoundBase* CloseSound = nullptr;

	bool  bWantsOpen = false;
	float OpenAlpha  = 0.f;   // 0 닫힘 ~ 1 열림
	FVector LeftClosed  = FVector::ZeroVector;
	FVector RightClosed = FVector::ZeroVector;
	FTimerHandle CloseTimerHandle;
};
