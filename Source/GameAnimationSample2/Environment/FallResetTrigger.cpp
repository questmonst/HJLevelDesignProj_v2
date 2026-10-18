// Copyright Epic Games, Inc. All Rights Reserved.

#include "FallResetTrigger.h"
#include "FallResetTarget.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "CheckpointSpotlight.h"

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
    // 1) 이 구역 전용 지점 → 2) 마지막 체크포인트 → 3) 플레이어 스타트
    FVector  Location = FVector::ZeroVector;
    FRotator Rotation = FRotator::ZeroRotator;
    bool bHasTarget = false;

    if (ResetTarget)
    {
        Location   = ResetTarget->GetActorLocation();
        Rotation   = ResetTarget->GetActorRotation();
        bHasTarget = true;
    }
    else if (const UCheckpointSubsystem* Checkpoints = GetWorld()->GetSubsystem<UCheckpointSubsystem>())
    {
        bHasTarget = Checkpoints->FindRespawnPoint(Player, PC, Location, Rotation);
    }

    if (bHasTarget)
    {
        const FRotator Facing(0.f, Rotation.Yaw, 0.f);
        Player->SetActorLocationAndRotation(Location, Facing, false, nullptr, ETeleportType::TeleportPhysics);
        if (PC) PC->SetControlRotation(Facing);   // 카메라도 리스폰 지점이 보는 쪽으로
    }

    if (bResetVelocity)
    {
        Player->GetCharacterMovement()->StopMovementImmediately();
    }

    if (FallDamage > 0.f)
    {
        UGameplayStatics::ApplyDamage(Player, FallDamage, nullptr, this, nullptr);
    }

    if (PC && FadeInDuration > 0.f)
    {
        PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeInDuration, FColor::Black);
    }

    bIsTeleporting = false;
}
