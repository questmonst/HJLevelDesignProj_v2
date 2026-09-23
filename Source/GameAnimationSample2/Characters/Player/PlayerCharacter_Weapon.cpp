// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"

float APlayerCharacter::PlayMontageForDuration(UAnimMontage* Montage)
{
	if (!Montage) return 0.f;
	const float Length = PlayAnimMontage(Montage);
	return Length / FMath::Max(Montage->RateScale, KINDA_SMALL_NUMBER);
}

void APlayerCharacter::SetCurrentWeaponHidden(bool bHideWeapon)
{
	if (CurrentWeapon) CurrentWeapon->SetActorHiddenInGame(bHideWeapon);
}

bool APlayerCharacter::PickupWeapon(AWeaponBase* Weapon)
{
	if (!Weapon) return false;
	if (WeaponInventory.Num() >= MaxWeaponSlots) return false;

	const bool bFromWorld = Weapon->IsDropped();

	WeaponInventory.Add(Weapon);
	Weapon->SetOwner(this);

	if (WeaponInventory.Num() == 1)
	{
		Weapon->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			WeaponAttachSocket);
		Weapon->RefreshMeshTransform();
		CurrentWeaponIndex = 0;
		CurrentWeapon      = Weapon;
	}
	else
	{
		const FName HolsterSocket = WeaponHolsterSocket.IsNone() ? WeaponAttachSocket : WeaponHolsterSocket;
		Weapon->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			HolsterSocket);
		Weapon->RefreshMeshTransform();
		Weapon->SetActorHiddenInGame(WeaponHolsterSocket.IsNone());
	}

	if (bFromWorld)
	{
		Weapon->SetDropped(false);
		if (USoundBase* Sound = Weapon->GetPickupSound())
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		}
	}

	return true;
}

void APlayerCharacter::OnWeaponPickupRangeEnter(AWeaponBase* Weapon)
{
	if (!Weapon || !Weapon->IsDropped()) return;
	if (bIsSwapping) return;

	if (WeaponInventory.Num() < MaxWeaponSlots)
	{
		PickupWeapon(Weapon);
	}
	else
	{
		PendingPickupWeapon = Weapon;
		bShowPickupPrompt   = true;
		OnPickupPromptChanged(true, Weapon);
	}
}

void APlayerCharacter::OnWeaponPickupRangeExit(AWeaponBase* Weapon)
{
	if (PendingPickupWeapon != Weapon) return;

	PendingPickupWeapon = nullptr;
	bShowPickupPrompt   = false;
	OnPickupPromptChanged(false, nullptr);
}

void APlayerCharacter::DropCurrentWeapon()
{
	if (!CurrentWeapon) return;

	CurrentWeapon->StopFire();

	AWeaponBase* DroppedWeapon = CurrentWeapon;
	WeaponInventory.RemoveAt(CurrentWeaponIndex);

	// 발 앞쪽에 드롭
	const FVector DropLocation = GetActorLocation()
		+ GetActorForwardVector() * 80.f
		- FVector(0.f, 0.f, 90.f);

	DroppedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	DroppedWeapon->SetActorLocation(DropLocation);
	DroppedWeapon->SetActorRotation(FRotator::ZeroRotator);
	DroppedWeapon->SetOwner(nullptr);
	DroppedWeapon->SetDropped(true); // 0.5초 후 구체 활성화

	// 다음 슬롯으로 전환
	if (WeaponInventory.Num() > 0)
	{
		CurrentWeaponIndex = FMath::Clamp(CurrentWeaponIndex, 0, WeaponInventory.Num() - 1);
		CurrentWeapon      = WeaponInventory[CurrentWeaponIndex];
		CurrentWeapon->SetActorHiddenInGame(false);
		CurrentWeapon->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			WeaponAttachSocket);
		CurrentWeapon->RefreshMeshTransform();
	}
	else
	{
		CurrentWeaponIndex = -1;
		CurrentWeapon      = nullptr;
	}
}

void APlayerCharacter::TryPickupNearbyWeapon()
{
	if (!PendingPickupWeapon) return;

	AWeaponBase* Weapon = PendingPickupWeapon;
	PendingPickupWeapon = nullptr;
	bShowPickupPrompt   = false;

	// 슬롯 꽉 차면: 현재 든 무기를 같은 슬롯에서 땅의 무기와 교체 (인덱스 유지)
	if (WeaponInventory.Num() >= MaxWeaponSlots)
	{
		SwapCurrentWeaponWith(Weapon);
		OnPickupPromptChanged(false, nullptr);
		return;
	}

	if (!PickupWeapon(Weapon))
	{
		PendingPickupWeapon = Weapon;
		bShowPickupPrompt   = true;
	}
	else
	{
		OnPickupPromptChanged(false, nullptr);
	}
}

void APlayerCharacter::SwapCurrentWeaponWith(AWeaponBase* NewWeapon)
{
	if (!NewWeapon || !CurrentWeapon || !WeaponInventory.IsValidIndex(CurrentWeaponIndex)) return;

	AWeaponBase* OldWeapon = CurrentWeapon;
	const bool   bFromWorld = NewWeapon->IsDropped();

	// 1) 현재 무기를 발 앞에 드롭 (DropCurrentWeapon과 동일한 드롭 처리, 단 인벤토리에서 제거하지 않고 교체)
	OldWeapon->StopFire();
	const FVector DropLocation = GetActorLocation()
		+ GetActorForwardVector() * 80.f
		- FVector(0.f, 0.f, 90.f);
	OldWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	OldWeapon->SetActorLocation(DropLocation);
	OldWeapon->SetActorRotation(FRotator::ZeroRotator);
	OldWeapon->SetOwner(nullptr);
	OldWeapon->SetDropped(true);

	// 2) 같은 슬롯에 새 무기 배치 + 즉시 장착
	WeaponInventory[CurrentWeaponIndex] = NewWeapon;
	CurrentWeapon = NewWeapon;
	NewWeapon->SetOwner(this);
	NewWeapon->SetActorHiddenInGame(false);
	NewWeapon->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		WeaponAttachSocket);
	NewWeapon->RefreshMeshTransform();

	if (bFromWorld)
	{
		NewWeapon->SetDropped(false);
		if (USoundBase* Sound = NewWeapon->GetPickupSound())
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		}
	}
}

void APlayerCharacter::OnPickupPromptChanged_Implementation(bool bShow, AWeaponBase* Weapon)
{
	// BP에서 오버라이드해서 HUD 위젯 표시/숨김 처리
}

void APlayerCharacter::EquipWeapon(int32 Index)
{
	if (!WeaponInventory.IsValidIndex(Index)) return;
	if (Index == CurrentWeaponIndex) return;
	if (bIsSwapping) return;

	bIsSwapping        = true;
	LastWeaponIndex    = CurrentWeaponIndex;
	PendingWeaponIndex = Index;

	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
		const FName HolsterSocket = WeaponHolsterSocket.IsNone() ? WeaponAttachSocket : WeaponHolsterSocket;
		CurrentWeapon->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			HolsterSocket);
		CurrentWeapon->RefreshMeshTransform();
		if (WeaponHolsterSocket.IsNone())
			CurrentWeapon->SetActorHiddenInGame(true);
	}

	GetWorldTimerManager().SetTimer(SwapTimerHandle, this, &APlayerCharacter::FinishEquipWeapon, WeaponSwapDelay, false);
}

void APlayerCharacter::FinishEquipWeapon()
{
	bIsSwapping = false;

	if (!WeaponInventory.IsValidIndex(PendingWeaponIndex)) return;

	CurrentWeaponIndex = PendingWeaponIndex;
	CurrentWeapon      = WeaponInventory[CurrentWeaponIndex];

	CurrentWeapon->SetActorHiddenInGame(false);
	CurrentWeapon->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		WeaponAttachSocket);
	CurrentWeapon->RefreshMeshTransform();
}

void APlayerCharacter::EquipWeaponSlot1() { EquipWeapon(0); }
void APlayerCharacter::EquipWeaponSlot2() { EquipWeapon(1); }
void APlayerCharacter::EquipWeaponSlot3() { EquipWeapon(2); }

void APlayerCharacter::SwapToLastWeapon()
{
	if (WeaponInventory.IsValidIndex(LastWeaponIndex))
		EquipWeapon(LastWeaponIndex);
}

UAnimMontage* APlayerCharacter::SelectFireMontage() const
{
	// 앉기 전용 몽타주가 지정돼 있을 때만 교체 — 비어 있으면 기존 동작 그대로
	if (bIsCrouched && FireMontageCrouch) return FireMontageCrouch;
	return FireMontage;
}

void APlayerCharacter::StartFire()
{
	if (!CurrentWeapon) return;
	if (bIsPreparingThrow || HeldGrenade) return;   // 수류탄 조준·투척 중엔 총을 숨겨 두므로 사격 불가
	bIsFiring = true;

	// 잔탄 없음·장전 중·단발 쿨다운이면 탄이 안 나가므로 반동 몽타주도 재생하지 않는다 (빈 총 소리만)
	const bool bWillFire = CurrentWeapon->IsFireReady();
	CurrentWeapon->StartFire();
	if (!bWillFire) return;

	ActiveFireMontage = SelectFireMontage();
	if (ActiveFireMontage)
		PlayAnimMontage(ActiveFireMontage);
}

void APlayerCharacter::OnWeaponShotFired()
{
	ApplyRecoilShot();
	if (CurrentWeapon && !CurrentWeapon->CanFire() && ActiveFireMontage)
	{
		StopAnimMontage(ActiveFireMontage);
		ActiveFireMontage = nullptr;
	}
}

void APlayerCharacter::StopFire()
{
	if (!CurrentWeapon) return;
	bIsFiring = false;
	CurrentWeapon->StopFire();
	// 자동화기만 릴리즈 시 몽타주 중단 (단발은 자연스럽게 끝남)
	if (CurrentWeapon->IsAutoFire())
	{
		if (ActiveFireMontage)
			StopAnimMontage(ActiveFireMontage);
	}
	ActiveFireMontage = nullptr;
}

void APlayerCharacter::Reload()
{
	if (!CurrentWeapon) return;

	// 실제로 장전이 시작됐을 때만 몽타주 (가득 참·예비탄 없음·이미 장전 중이면 무시)
	const bool bWasReloading = CurrentWeapon->IsReloading();
	CurrentWeapon->Reload();
	if (!bWasReloading && CurrentWeapon->IsReloading() && ReloadMontage)
	{
		// 무기마다 장전 시간이 달라서, 몽타주 재생 길이를 그 무기의 ReloadTime에 맞춘다
		const float BaseDuration = ReloadMontage->GetPlayLength() / FMath::Max(ReloadMontage->RateScale, KINDA_SMALL_NUMBER);
		const float ReloadTime   = CurrentWeapon->GetReloadTime();
		const float PlayRate     = (ReloadTime > 0.f) ? BaseDuration / ReloadTime : 1.f;
		PlayAnimMontage(ReloadMontage, PlayRate);
	}

	// 장전이 실제로 시작됐으면 총을 왼손으로 옮겼다가 장전 시간 뒤 되돌린다
	if (!bWasReloading && CurrentWeapon->IsReloading() && bAttachWeaponToLeftHandOnReload)
	{
		AttachWeaponToLeftHand();
		const float ReloadTime = FMath::Max(CurrentWeapon->GetReloadTime(), 0.01f);
		GetWorldTimerManager().SetTimer(ReloadAttachTimerHandle, this,
			&APlayerCharacter::RestoreWeaponToRightHand, ReloadTime, false);
	}
}

void APlayerCharacter::AttachWeaponToLeftHand()
{
	if (!CurrentWeapon || !GetMesh() || !GetMesh()->DoesSocketExist(ReloadLeftHandSocket)) return;

	// 돌아갈 때 그대로 되돌리기 위해 지금 상태를 기억해둔다
	PreReloadWeaponTransform = CurrentWeapon->GetRootComponent()->GetRelativeTransform();

	CurrentWeapon->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale, ReloadLeftHandSocket);

	// 무기 루트가 아니라 LeftHandGrip 소켓이 손에 오도록, 그 소켓의 역트랜스폼만큼 되민다.
	// 배율은 빼고 위치·회전만 쓴다 — 메시 배율까지 뒤집으면 총이 작아지거나 커진다
	FTransform GripLocal = CurrentWeapon->GetLeftHandGripTransform()
		.GetRelativeTransform(CurrentWeapon->GetActorTransform());
	GripLocal.SetScale3D(FVector::OneVector);

	FTransform NewRelative = GripLocal.Inverse();
	NewRelative.SetScale3D(PreReloadWeaponTransform.GetScale3D());   // 원래 배율 유지
	CurrentWeapon->SetActorRelativeTransform(NewRelative);

	bWeaponInLeftHand = true;
}

void APlayerCharacter::RestoreWeaponToRightHand()
{
	if (!bWeaponInLeftHand) return;
	bWeaponInLeftHand = false;

	if (!CurrentWeapon || !GetMesh()) return;
	CurrentWeapon->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);
	CurrentWeapon->SetActorRelativeTransform(PreReloadWeaponTransform);   // 장전 전 상태 그대로
}
