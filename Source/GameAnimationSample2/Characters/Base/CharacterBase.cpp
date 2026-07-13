// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/DamageEvents.h"
#include "DamageNumberActor.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxHealth = 100.0f;
	CurrentHealth = 100.0f;
	TeamID = 0;

	// 무기 히트스캔은 Visibility 채널로 트레이스한다. 기본 Pawn 프로파일은
	// Visibility=Ignore라 총알이 캐릭터를 통과하므로, 피격 판정을 위해 캡슐이
	// Visibility를 Block하도록 강제한다. (모든 캐릭터=플레이어·적 공통)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 머리 위 체력바 (기본 숨김, 적 클래스에서 bShowFloatingHealthBar로 활성화)
	HealthBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarWidgetComp->SetupAttachment(GetCapsuleComponent());
	HealthBarWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	HealthBarWidgetComp->SetDrawAtDesiredSize(true);
	HealthBarWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, HealthBarHeightOffset));
	HealthBarWidgetComp->SetVisibility(false);
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	if (bShowFloatingHealthBar && HealthBarWidgetClass)
	{
		HealthBarWidgetComp->SetWidgetClass(HealthBarWidgetClass);
	}
	HealthBarWidgetComp->SetVisibility(false);
}

void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateHealthBar(DeltaTime);
}

float ACharacterBase::GetHealthPercent() const
{
	return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f;
}

void ACharacterBase::TakeDamageCustom_Implementation(float Amount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

// --- Floating Health Bar / Damage Number ---

void ACharacterBase::ShowHealthBar()
{
	if (!bShowFloatingHealthBar) return;

	bHealthBarActive = true;
	GetWorldTimerManager().SetTimer(
		HealthBarHideTimer, this, &ACharacterBase::HideHealthBar, HealthBarHideDelay, false);
}

void ACharacterBase::HideHealthBar()
{
	bHealthBarActive = false;
}

void ACharacterBase::UpdateHealthBar(float /*DeltaTime*/)
{
	if (!bShowFloatingHealthBar || !HealthBarWidgetComp) return;

	bool bShouldShow = bHealthBarActive;

	if (bShouldShow)
	{
		if (APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			const FVector BarLoc = HealthBarWidgetComp->GetComponentLocation();
			const FVector CamLoc = Cam->GetCameraLocation();

			if (HealthBarMaxDrawDistance > 0.f && FVector::Dist(CamLoc, BarLoc) > HealthBarMaxDrawDistance)
			{
				bShouldShow = false;   // 너무 멀면 이번 프레임엔 숨김 (재접근 시 다시 표시)
			}
			else
			{
				// 카메라를 향해 Yaw 빌보드. 글자가 뒤집히면 +180.f 제거.
				const FVector ToCam = CamLoc - BarLoc;
				HealthBarWidgetComp->SetWorldRotation(FRotator(0.f, ToCam.Rotation().Yaw + 180.f, 0.f));
			}
		}
	}

	HealthBarWidgetComp->SetVisibility(bShouldShow);
}

void ACharacterBase::SpawnDamageNumber(float Amount, const FVector& WorldLocation)
{
	if (!DamageNumberActorClass) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = this;

	ADamageNumberActor* Floater = GetWorld()->SpawnActor<ADamageNumberActor>(
		DamageNumberActorClass, WorldLocation, FRotator::ZeroRotator, Params);
	if (Floater)
	{
		Floater->InitDamage(Amount);
	}
}

void ACharacterBase::OnDeath_Implementation()
{
	// Default: destroy actor. Override in Blueprint for death animations, ragdoll, etc.
	Destroy();
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                  AController* EventInstigator, AActor* DamageCauser)
{
	float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (Actual <= 0.f)
	{
		return Actual;
	}

	// 피격 위치: 점 피해면 실제 탄착점, 아니면 액터 위치
	FVector HitLoc = GetActorLocation();
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		HitLoc = static_cast<const FPointDamageEvent&>(DamageEvent).HitInfo.ImpactPoint;
	}

	// 피드백은 체력 감소(파괴 가능)보다 먼저 — this가 아직 유효할 때 스폰.
	// 플로터는 독립 액터라 이 캐릭터가 곧 Destroy돼도 그대로 남는다.
	SpawnDamageNumber(Actual, HitLoc);
	ShowHealthBar();
	OnDamaged.Broadcast(Actual, HitLoc);

	TakeDamageCustom(Actual);   // 체력 감소 + OnHealthChanged + (사망 시) Die
	return Actual;
}

void ACharacterBase::Die()
{
	if (bIsDead) return;
	bIsDead = true;
	OnDeath();
}
