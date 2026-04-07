// Copyright Epic Games, Inc. All Rights Reserved.

#include "FallResetTrigger.h"
#include "FallResetTarget.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

AFallResetTrigger::AFallResetTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    SetRootComponent(TriggerBox);

    TriggerBox->SetBoxExtent(FVector(200.f, 200.f, 50.f));
    TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    TriggerBox->SetGenerateOverlapEvents(true);
}

void AFallResetTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFallResetTrigger::OnTriggerOverlap);
}

void AFallResetTrigger::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsTeleporting) return;
    if (!ResetTarget)   return;

    ACharacter* Player = Cast<ACharacter>(OtherActor);
    if (!Player) return;

    // 플레이어 컨트롤러가 있는 캐릭터만 처리 (AI 제외)
    APlayerController* PC = Cast<APlayerController>(Player->GetController());
    if (!PC) return;

    bIsTeleporting = true;

    if (FadeOutDuration > 0.f)
    {
        PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, FadeOutDuration, FColor::Black, false, true);

        FTimerDelegate Delegate;
        Delegate.BindUObject(this, &AFallResetTrigger::DoTeleport, Player, PC);
        GetWorldTimerManager().SetTimer(TeleportTimerHandle, Delegate, FadeOutDuration, false);
    }
    else
    {
        DoTeleport(Player, PC);
    }
}

void AFallResetTrigger::DoTeleport(ACharacter* Player, APlayerController* PC)
{
    if (ResetTarget)
    {
        Player->SetActorTransform(
            ResetTarget->GetActorTransform(), false, nullptr, ETeleportType::TeleportPhysics);
    }

    if (bResetVelocity)
    {
        Player->GetCharacterMovement()->StopMovementImmediately();
    }

    if (PC && FadeInDuration > 0.f)
    {
        PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeInDuration, FColor::Black);
    }

    bIsTeleporting = false;
}
