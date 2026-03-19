// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AWeaponBase;
class AGrenadeBase;
class UInputMappingContext;
class UInputAction;
class USplineComponent;
class USplineMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class GAMEANIMATIONSAMPLE2_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	UCameraComponent* CameraComponent;

	// --- Input ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Input")
	UInputMappingContext* IMC_Default;

	// --- Input Handlers ---

	const UInputAction* FindActionInIMC(const FString& NameContains) const;
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void StartCrouch();
	void StopCrouch();

	// --- Movement ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement")
	float SprintSpeed;

	// --- Aim ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float NormalFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float AimFOV;

	bool bIsAiming;

	// --- Cover Peek ---

	/** 좌우 엄폐물 감지 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float CoverTraceDistance = 80.f;

	/** 엄폐 시 카메라 오프셋 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float CoverPeekOffset = 80.f;

	/** 카메라 오프셋 보간 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float CoverPeekInterpSpeed = 8.f;

	/** 기본 카메라 좌우 오프셋 (양수 = 오른쪽) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
	float DefaultSocketOffsetY = 60.f;

	float NormalSocketOffsetY = 0.f;

	void UpdateCoverPeek(float DeltaTime);

	// --- Grenade ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade")
	TSubclassOf<AGrenadeBase> GrenadeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade")
	float GrenadeThrowSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade")
	UStaticMesh* TrajectoryMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade")
	UMaterialInterface* TrajectoryMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Grenade")
	float TrajectoryMeshScale = 0.1f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Grenade")
	USplineComponent* TrajectorySpline;

	bool bIsPreparingThrow = false;

	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshPool;

	static const int32 MaxTrajectorySegments = 20;

	void StartGrenadeThrow();
	void ReleaseGrenadeThrow();
	void UpdateTrajectory();
	void ClearTrajectory();

	// --- Weapon ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon")
	TArray<AWeaponBase*> WeaponInventory;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon")
	AWeaponBase* CurrentWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Weapon")
	int32 CurrentWeaponIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon")
	int32 MaxWeaponSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Weapon")
	FName WeaponAttachSocket;

public:
	// --- Movement ---

	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void StopSprint();

	// --- Aim ---

	UFUNCTION(BlueprintCallable, Category = "Character|Camera")
	void StartAim();

	UFUNCTION(BlueprintCallable, Category = "Character|Camera")
	void StopAim();

	// --- Weapon ---

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	virtual void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	virtual void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	void EquipWeapon(int32 Index);

	void EquipWeaponSlot1();
	void EquipWeaponSlot2();
	void EquipWeaponSlot3();

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	bool PickupWeapon(AWeaponBase* Weapon);

	UFUNCTION(BlueprintPure, Category = "Character|Weapon")
	AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }
};
