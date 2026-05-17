// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "WeaponBase.h"

// WeaponBase::Fire()에서 호출. 발사 1회당 반동 누적 + 즉시 카메라 kick.
void APlayerCharacter::ApplyRecoilShot()
{
	if (!CurrentWeapon) return;

	const float Pitch    = CurrentWeapon->GetRecoilPitch();
	const float YawRange = CurrentWeapon->GetRecoilYawRange();
	const float MaxPitch = CurrentWeapon->GetMaxRecoilPitch();

	// 상한선 넘지 않도록 실제 적용량 계산
	const float ActualPitch = FMath::Min(Pitch, MaxPitch - RecoilPitchAccum);
	if (ActualPitch <= 0.f) return;

	RecoilPitchAccum += ActualPitch;

	// 음수 = 카메라 위로 kick (UE 컨벤션)
	AddControllerPitchInput(-ActualPitch);
	AddControllerYawInput(FMath::FRandRange(-YawRange, YawRange));
}

// Tick에서 호출. 사격 여부와 무관하게 항상 회복.
// 자동화기: 발사 속도 > 회복 속도면 자연히 누적, 멈추면 회복.
void APlayerCharacter::UpdateRecoil(float DeltaTime)
{
	if (RecoilPitchAccum <= 0.f || !CurrentWeapon || bIsFiring) return;

	const float RecoveryDelta = FMath::Min(
		CurrentWeapon->GetRecoilRecoverySpeed() * DeltaTime,
		RecoilPitchAccum);

	RecoilPitchAccum -= RecoveryDelta;
	AddControllerPitchInput(RecoveryDelta); // 양수 = 카메라 아래로 (회복)
}
