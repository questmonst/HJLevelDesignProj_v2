// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/DamageEvents.h"
#include "DamageNumberActor.h"
#include "EnemyHealthBarWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 펀치 범위 데칼 등은 벽·바닥에만 — 캐릭터 몸에는 그리지 않는다
	GetMesh()->SetReceivesDecals(false);

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
	// Screen 스페이스 — 항상 지오메트리 위에 그려져 벽·캐릭터에 파묻히지 않고,
	// 위젯 크기가 월드 cm가 아니라 픽셀로 해석된다. (플레이어 UI로 취급)
	HealthBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
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

		// WidgetComponent가 만든 위젯은 자기를 띄운 액터를 알 수 없다(GetOwningPlayerPawn은 플레이어).
		// 여기서 인스턴스를 즉시 만들어 소유자를 주입해야 WBP가 체력을 읽을 수 있다.
		HealthBarWidgetComp->InitWidget();
		if (UEnemyHealthBarWidget* Bar = Cast<UEnemyHealthBarWidget>(HealthBarWidgetComp->GetUserWidgetObject()))
		{
			Bar->BindToCharacter(this);
		}
	}

	HealthBarAlpha = 0.f;
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

float ACharacterBase::Heal(float Amount)
{
	if (bIsDead || Amount <= 0.f) return 0.f;

	const float Before = CurrentHealth;
	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
	const float Healed = CurrentHealth - Before;

	if (Healed > 0.f)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
	return Healed;
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

	// 위젯이 새 인스턴스로 재생성됐다면 소유자 주입이 통째로 날아간 상태다.
	// 피격마다 다시 주입해둔다(BindToCharacter는 중복 구독을 스스로 정리한다).
	if (HealthBarWidgetComp)
	{
		if (UEnemyHealthBarWidget* Bar = Cast<UEnemyHealthBarWidget>(HealthBarWidgetComp->GetUserWidgetObject()))
		{
			Bar->BindToCharacter(this);
		}
	}

	bHealthBarActive = true;
	GetWorldTimerManager().SetTimer(
		HealthBarHideTimer, this, &ACharacterBase::HideHealthBar, HealthBarHideDelay, false);
}

void ACharacterBase::HideHealthBar()
{
	bHealthBarActive = false;
}

void ACharacterBase::UpdateHealthBar(float DeltaTime)
{
	if (!bShowFloatingHealthBar || !HealthBarWidgetComp) return;

	// bHealthBarActive(최근 피격/사망) + 거리 + 화면 안 조건을 매 프레임 재평가해
	// 목표 알파를 정하고, 그 값으로 보간한다. (즉시 껐다 켜지 않고 페이드)
	// → 화면 밖으로 나가면 사라지고, 다시 들어오면(피격 후 유지시간 내) 다시 나타난다.
	bool bShouldShow = bHealthBarActive;
	float DesiredScale = 1.f;

	if (bShouldShow)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (!PC)
		{
			ApplyHealthBarAlpha(0.f);
			return;
		}

		const FVector BarLoc = HealthBarWidgetComp->GetComponentLocation();
		FVector CamLoc; FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		const float CamDist = FVector::Dist(CamLoc, BarLoc);

		// 거리 체크
		if (HealthBarMaxDrawDistance > 0.f && CamDist > HealthBarMaxDrawDistance)
		{
			bShouldShow = false;
		}
		else
		{
			// 화면(프러스텀) 안 체크 — 뷰포트 밖이거나 카메라 뒤면 숨김
			FVector2D ScreenPos;
			int32 VX = 0, VY = 0;
			PC->GetViewportSize(VX, VY);
			const bool bOnScreen =
				PC->ProjectWorldLocationToScreen(BarLoc, ScreenPos, false)
				&& ScreenPos.X >= 0.f && ScreenPos.X <= VX
				&& ScreenPos.Y >= 0.f && ScreenPos.Y <= VY;

			if (!bOnScreen)
			{
				bShouldShow = false;
			}
			else if (HealthBarRefDistance > 0.f)
			{
				// Screen 스페이스는 거리와 무관하게 같은 픽셀 크기라, 먼 적의 체력바까지
				// 크게 떠서 화면이 지저분해진다. 거리에 따라 줄이되 MinScale로 하한을 둬
				// 가독성을 보장한다. (MinScale=1.0이면 항상 같은 크기)
				DesiredScale = FMath::Clamp(
					HealthBarRefDistance / FMath::Max(CamDist, 1.f), HealthBarMinScale, 1.f);
			}
		}
	}

	// 나타날 때는 즉시 — 피격 피드백이 늦으면 타격감이 죽고, 대미지 숫자가 즉시 뜨는 것과
	// 어긋나 체력 감소가 느린 것처럼 보인다. 페이드는 사라질 때만 적용한다.
	// (마지막 피격 후 HealthBarHideDelay=3초 경과 → HealthBarFadeDuration 동안 서서히)
	const float TargetAlpha = bShouldShow ? 1.f : 0.f;
	if (TargetAlpha > HealthBarAlpha || HealthBarFadeDuration <= 0.f)
	{
		HealthBarAlpha = TargetAlpha;
	}
	else
	{
		HealthBarAlpha = FMath::FInterpConstantTo(
			HealthBarAlpha, TargetAlpha, DeltaTime, 1.f / HealthBarFadeDuration);
	}

	if (bShouldShow)
	{
		if (UUserWidget* Bar = HealthBarWidgetComp->GetUserWidgetObject())
		{
			Bar->SetRenderScale(FVector2D(DesiredScale, DesiredScale));
		}
	}

	ApplyHealthBarAlpha(HealthBarAlpha);
}

// 알파를 위젯 RenderOpacity에 반영하고, 완전히 투명하면 렌더에서 뺀다.
void ACharacterBase::ApplyHealthBarAlpha(float Alpha)
{
	if (!HealthBarWidgetComp) return;

	HealthBarAlpha = Alpha;

	if (UUserWidget* Bar = HealthBarWidgetComp->GetUserWidgetObject())
	{
		Bar->SetRenderOpacity(Alpha);
	}
	HealthBarWidgetComp->SetVisibility(Alpha > UE_KINDA_SMALL_NUMBER);
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
	// 체력바를 강제로 켜둔 채(숨김 타이머 취소, 알파 1) 사망 연출 재생 시간을 확보한다.
	if (bShowFloatingHealthBar && HealthBarWidgetComp)
	{
		GetWorldTimerManager().ClearTimer(HealthBarHideTimer);
		bHealthBarActive = true;
		ApplyHealthBarAlpha(1.f);

		// 위젯의 Anim_Death 재생 (BindWidgetAnim으로 자동 연결 — BP 배선 불필요)
		if (UEnemyHealthBarWidget* Bar = Cast<UEnemyHealthBarWidget>(HealthBarWidgetComp->GetUserWidgetObject()))
		{
			Bar->PlayDeathEffect();
		}
	}

	OnDeathEffect();   // BP: 체력바 위젯의 확대·소멸 애니 재생

	// 렉돌을 쓰면 제거 시점은 렉돌 흐름이 정한다 (화면 밖으로 나가면 사라짐)
	if (bRagdollOnDeath)
	{
		// 몽타주가 실제로 재생됐을 때만 기다린다. 슬롯이 없어 재생이 안 됐는데 기다리면
		// 아무 모션 없이 멍하니 서 있다가 뒤늦게 쓰러진다
		const float MontageLength = PlayRandomMontage(DeathMontages, 0.1f);
		const float Delay = MontageLength * FMath::Clamp(RagdollDelayRate, 0.f, 1.f);
		if (Delay > 0.f)
		{
			GetWorldTimerManager().SetTimer(
				RagdollTimerHandle, this, &ACharacterBase::BeginDeathRagdoll, Delay, false);
		}
		else
		{
			BeginDeathRagdoll();
		}
		return;
	}

	// 연출 시간 후 실제 제거. (즉시 Destroy하면 연출이 재생될 틈이 없음)
	if (DeathEffectDuration > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			DeathDestroyTimer, this, &ACharacterBase::FinishDeath, DeathEffectDuration, false);
	}
	else
	{
		Destroy();
	}
}

void ACharacterBase::FinishDeath()
{
	Destroy();
}

float ACharacterBase::PlayRandomMontage(const TArray<UAnimMontage*>& Montages, float BlendTime)
{
	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim || Montages.Num() == 0) return 0.f;

	// 비어 있는 칸이 섞여 있을 수 있으니 유효한 것만 모아서 고른다
	TArray<UAnimMontage*> Valid;
	Valid.Reserve(Montages.Num());
	for (UAnimMontage* M : Montages)
	{
		if (M) Valid.Add(M);
	}
	if (Valid.Num() == 0) return 0.f;

	UAnimMontage* Picked = Valid[FMath::RandRange(0, Valid.Num() - 1)];
	Anim->Montage_PlayWithBlendIn(Picked, FAlphaBlendArgs(BlendTime));

	// 슬롯이 ABP에 없으면 재생되지 않는다 — 그걸 길이로 착각하면 아무것도 안 나오는 채로 기다리게 된다
	if (!Anim->Montage_IsPlaying(Picked)) return 0.f;

	return Picked->GetPlayLength() / FMath::Max(Picked->RateScale, KINDA_SMALL_NUMBER);
}

void ACharacterBase::PlayHitReactMontage()
{
	if (bIsDead || bIsRagdoll) return;
	if (GetWorld()->TimeSince(LastHitMontageTime) < HitMontageMinInterval) return;
	if (PlayRandomMontage(HitMontages, HitMontageBlendTime) > 0.f)
	{
		LastHitMontageTime = GetWorld()->GetTimeSeconds();
	}
}

// --- Ragdoll ---

void ACharacterBase::EnterRagdoll()
{
	if (bIsRagdoll) return;

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;

	bIsRagdoll       = true;
	RagdollStartTime = GetWorld()->GetTimeSeconds();
	MeshRelativeTransformBeforeRagdoll = MeshComp->GetRelativeTransform();

	// 몽타주가 계속 돌면 물리와 싸운다
	if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
	{
		Anim->StopAllMontages(0.1f);
	}

	// 캡슐은 비켜준다 — 안 그러면 렉돌이 자기 캡슐에 걸려 튄다
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetAllBodiesSimulatePhysics(true);
	MeshComp->WakeAllRigidBodies();
	MeshComp->bBlendPhysics = true;
}

void ACharacterBase::KnockdownToRagdoll(const FVector& Impulse)
{
	if (bIsDead || bIsRagdoll || !bRagdollOnKnockback) return;

	EnterRagdoll();
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		// 모든 바디에 속도를 줘야 한다. 골반 하나만 밀면 나머지 뼈가 정지 상태로 붙잡고 늘어져
		// 운동량이 관절을 늘리는 데 소모되고, 몸은 거의 날아가지 않는다
		MeshComp->SetAllPhysicsLinearVelocity(Impulse);
	}

	// 정해진 시간이 아니라 "멈출 때까지" 누워 있는다. 0.1초마다 속도를 본다
	GetWorldTimerManager().SetTimer(
		RagdollTimerHandle, this, &ACharacterBase::TickKnockbackRagdoll, 0.1f, true, KnockbackRagdollMinTime);
}

void ACharacterBase::TickKnockbackRagdoll()
{
	if (!bIsRagdoll || bIsDead)
	{
		GetWorldTimerManager().ClearTimer(RagdollTimerHandle);
		return;
	}

	const float Elapsed = GetWorld()->TimeSince(RagdollStartTime);
	bool bDone = Elapsed >= KnockbackRagdollMaxTime;   // 계속 굴러떨어져도 언젠가는 일어난다

	if (!bDone)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			const float Speed = MeshComp->GetPhysicsLinearVelocity(RagdollPelvisBone).Size();
			bDone = Speed <= KnockbackSettleSpeed;
		}
	}

	if (bDone)
	{
		GetWorldTimerManager().ClearTimer(RagdollTimerHandle);
		ExitRagdollAndGetUp();
	}
}

void ACharacterBase::ExitRagdollAndGetUp()
{
	if (!bIsRagdoll) return;
	if (bIsDead) return;   // 렉돌로 구르는 중에 죽었으면 그대로 시체

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		bIsRagdoll = false;
		return;
	}

	// 캡슐은 쓰러진 자리에 그대로 남아 있다. 골반 아래 바닥을 찾아 캡슐을 옮기지 않으면
	// 물리를 끄는 순간 메시가 캡슐 자리로 순간이동한다
	const FVector PelvisLoc  = MeshComp->GetBoneLocation(RagdollPelvisBone);
	const float   HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FVector TargetLoc = PelvisLoc + FVector(0.f, 0.f, HalfHeight);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(
			Hit, PelvisLoc, PelvisLoc - FVector(0.f, 0.f, 500.f), ECC_Visibility, Params))
	{
		TargetLoc = Hit.ImpactPoint + FVector(0.f, 0.f, HalfHeight);
	}

	MeshComp->SetAllBodiesSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetCollisionProfileName(TEXT("CharacterMesh"));
	MeshComp->bBlendPhysics = false;
	MeshComp->SetRelativeTransform(MeshRelativeTransformBeforeRagdoll);

	SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	bIsRagdoll = false;

	// 일어나는 동안에도 공격은 막는다 (BT는 bIsRagdoll만 보면 기상 순간부터 다시 쏜다)
	if (GetUpMontage)
	{
		const float Length = PlayRandomMontage({ GetUpMontage }, 0.1f);
		if (Length > 0.f)
		{
			bIsGettingUp = true;
			GetWorldTimerManager().SetTimer(GetUpTimerHandle, this, &ACharacterBase::EndGetUp, Length, false);
		}
	}
}

void ACharacterBase::BeginDeathRagdoll()
{
	GetWorldTimerManager().ClearTimer(RagdollTimerHandle);   // 넉백 기상 예약이 있으면 취소
	EnterRagdoll();

	// 최소 시간이 지난 뒤부터, 화면 밖으로 나갔는지 주기적으로 확인
	GetWorldTimerManager().SetTimer(
		CorpseDespawnTimerHandle, this, &ACharacterBase::TickCorpseDespawn, 0.5f, true, CorpseMinTime);
}

void ACharacterBase::TickCorpseDespawn()
{
	// 계속 보고 있어도 CorpseMaxTime이 지나면 정리한다 (시체가 영원히 쌓이지 않게)
	const bool bTimedOut = GetWorld()->TimeSince(RagdollStartTime) >= CorpseMaxTime;
	if (bTimedOut || !WasRecentlyRendered(0.5f))
	{
		GetWorldTimerManager().ClearTimer(CorpseDespawnTimerHandle);
		Destroy();
	}
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
	PlayHitReactMontage();   // 사망·렉돌 중이면 내부에서 무시

	TakeDamageCustom(Actual);   // 체력 감소 + OnHealthChanged + (사망 시) Die
	return Actual;
}

void ACharacterBase::Die()
{
	if (bIsDead) return;
	bIsDead = true;
	OnDeath();
}
