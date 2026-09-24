// Copyright Epic Games, Inc. All Rights Reserved.

#include "CrosshairWidget.h"
#include "Animation/WidgetAnimation.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!bAutoBindToPlayer) return;

	if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Player->OnHitConfirmed.AddUniqueDynamic(this, &UCrosshairWidget::HandleHitConfirmed);
		Player->OnEnemyKilled.AddUniqueDynamic(this, &UCrosshairWidget::HandleEnemyKilled);
	}
}

void UCrosshairWidget::NativeDestruct()
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Player->OnHitConfirmed.RemoveDynamic(this, &UCrosshairWidget::HandleHitConfirmed);
		Player->OnEnemyKilled.RemoveDynamic(this, &UCrosshairWidget::HandleEnemyKilled);
	}
	Super::NativeDestruct();
}

void UCrosshairWidget::HandleHitConfirmed(bool bHeadshot)
{
	if (bHeadshot) PlayHeadShotMarker();
	else           PlayHitMarker();
}

void UCrosshairWidget::HandleEnemyKilled()
{
	PlayKillMarker();
}

void UCrosshairWidget::PlayHitMarker()
{
	// 같은 애니를 연달아 맞을 수 있으므로 처음부터 다시 재생한다
	if (Anim_Hit) PlayAnimation(Anim_Hit, 0.f, 1);
}

void UCrosshairWidget::PlayHeadShotMarker()
{
	if (Anim_HeadShot) PlayAnimation(Anim_HeadShot, 0.f, 1);
	else if (Anim_Hit) PlayAnimation(Anim_Hit, 0.f, 1);
}

void UCrosshairWidget::PlayKillMarker()
{
	if (Anim_Killed) PlayAnimation(Anim_Killed, 0.f, 1);
}
