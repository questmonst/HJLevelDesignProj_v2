// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelInteraction.h"
#include "LevelInteractableBase.generated.h"

class UBoxComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UInteractColorData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractableChangedSignature, AActor*, Actor, bool, bInteractable);

// 스위치·문 공용 바탕 — "사용 가능(청색) / 불가(적색)" 상태, 상태 조명, 플레이어 감지 영역.
//
// 조명은 상태와 별개로 넣을지 말지 고른다(bUseStatusLight). 켜 두면 포인트 라이트 색과
// StatusLampMesh 머티리얼의 색 파라미터가 상태에 따라 바뀐다.
// 상태는 SetInteractable로 바꾸거나, 다른 액터의 레벨 신호(ReceiveLevelSignal)로 바뀐다.
UCLASS(Abstract)
class GAMEANIMATIONSAMPLE2_API ALevelInteractableBase : public AActor, public IInteractable, public ILevelSignalReceiver
{
	GENERATED_BODY()

public:
	ALevelInteractableBase();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractable(bool bNewInteractable);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsInteractable() const { return bInteractable; }

	// 청/적 상태가 바뀔 때
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableChangedSignature OnInteractableChanged;

	// ILevelSignalReceiver — 기본: 신호로 청/적 전환
	virtual void ReceiveLevelSignal(bool bOn, AActor* Source) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta=(ToolTip="플레이어가 이 안에 들어오면 반응한다 (스위치·수동문: 안내 표시, 자동문: 열림). 크기는 BP에서 조정"))
	UBoxComponent* InteractZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Light", meta=(ToolTip="상태 조명 (청/적)"))
	UPointLightComponent* StatusLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Light", meta=(ToolTip="상태 표시등 메시. 머티리얼에 StatusColorParam 벡터 파라미터가 있으면 색이 바뀐다"))
	UStaticMeshComponent* StatusLampMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta=(ToolTip="시작 상태. true = 청색(사용 가능), false = 적색(불가)"))
	bool bInteractable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Light", meta=(ToolTip="상태 조명·표시등을 쓸지. 끄면 상태는 그대로 동작하고 조명만 숨긴다"))
	bool bUseStatusLight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Light", meta=(ToolTip="상태 색 DA (DA_InteractColor). 지정하면 아래 두 색을 덮어쓴다"))
	UInteractColorData* ColorData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Light", meta=(ToolTip="사용 가능(청색) 색. ColorData에서 설정"))
	FLinearColor InteractableColor = FLinearColor(0.1f, 0.45f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Light", meta=(ToolTip="사용 불가(적색) 색. ColorData에서 설정"))
	FLinearColor LockedColor = FLinearColor(1.f, 0.08f, 0.05f);

	void ApplyColorData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Light", meta=(ToolTip="표시등 머티리얼의 색 벡터 파라미터 이름. 없으면 포인트 라이트만 바뀐다"))
	FName StatusColorParam = TEXT("Color");

	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> LampMaterials;

	void ApplyStatusVisuals();

	// 영역 드나듦 — 기본: 플레이어 상호작용 후보 등록/해제. 자동문은 덮어써서 열고 닫는다
	UFUNCTION()
	void HandleZoneBegin(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);
	UFUNCTION()
	void HandleZoneEnd(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex);

	virtual void OnPlayerEnterZone(APlayerCharacter* Player);
	virtual void OnPlayerLeaveZone(APlayerCharacter* Player);
	virtual void OnInteractableStateChanged() {}

	// 영역 안에 플레이어가 있는지 (자동문이 청색으로 바뀌는 순간 열지 판단)
	bool IsPlayerInZone() const;
};
