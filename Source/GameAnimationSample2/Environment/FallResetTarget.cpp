// Copyright Epic Games, Inc. All Rights Reserved.

#include "FallResetTarget.h"
#include "Components/ArrowComponent.h"

AFallResetTarget::AFallResetTarget()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

#if WITH_EDITORONLY_DATA
    DirectionArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
    if (DirectionArrow)
    {
        DirectionArrow->SetupAttachment(SceneRoot);
        DirectionArrow->ArrowColor     = FColor::Green;
        DirectionArrow->ArrowSize      = 2.f;
        DirectionArrow->bIsScreenSizeScaled = true;
    }
#endif
}
