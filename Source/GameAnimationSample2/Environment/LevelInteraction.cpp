// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelInteraction.h"

void LevelSignal::Send(const TArray<TObjectPtr<AActor>>& Targets, bool bOn, AActor* Source)
{
	for (AActor* Target : Targets)
	{
		if (ILevelSignalReceiver* Receiver = Cast<ILevelSignalReceiver>(Target))
		{
			Receiver->ReceiveLevelSignal(bOn, Source);
		}
	}
}
