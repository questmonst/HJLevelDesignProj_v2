// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"

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

void APlayerCharacter::StartFire()
{
	if (!CurrentWeapon) return;
	bIsFiring = true;
	CurrentWeapon->StartFire();
	if (FireMontage)
		PlayAnimMontage(FireMontage);
}

void APlayerCharacter::StopFire()
{
	if (!CurrentWeapon) return;
	bIsFiring = false;
	CurrentWeapon->StopFire();
	// 자동화기만 릴리즈 시 몽타주 중단 (단발은 자연스럽게 끝남)
	if (CurrentWeapon->IsAutoFire())
	{
		if (FireMontage)
			StopAnimMontage(FireMontage);
	}
}

void APlayerCharacter::Reload()
{
	if (CurrentWeapon) CurrentWeapon->Reload();
}
