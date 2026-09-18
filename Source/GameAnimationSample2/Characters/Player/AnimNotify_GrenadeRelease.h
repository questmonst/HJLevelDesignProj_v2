// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_GrenadeRelease.generated.h"

// 던지기 몽타주에서 "수류탄이 손을 떠나는 프레임"에 놓는 노티파이.
//
// 버튼을 떼는 순간 던지면 팔이 아직 뒤에 있는데 수류탄이 먼저 날아간다. 발사 시점을 애니메이터가
// 몽타주 타임라인에서 직접 정하게 해서 손동작과 투척을 맞춘다. 던지기 사운드도 이 순간 재생된다.
UCLASS(meta=(DisplayName="Grenade Release"))
class GAMEANIMATIONSAMPLE2_API UAnimNotify_GrenadeRelease : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                    const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
