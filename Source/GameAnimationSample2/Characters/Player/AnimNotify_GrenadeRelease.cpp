// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimNotify_GrenadeRelease.h"
#include "PlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_GrenadeRelease::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// 에디터 몽타주 프리뷰에서도 호출되므로 플레이어 캐릭터일 때만 동작
	if (APlayerCharacter* Player = MeshComp ? Cast<APlayerCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Player->LaunchHeldGrenade();
	}
}

FString UAnimNotify_GrenadeRelease::GetNotifyName_Implementation() const
{
	return TEXT("Grenade Release");
}
