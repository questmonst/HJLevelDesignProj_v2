// Copyright Epic Games, Inc. All Rights Reserved.

#include "DamageNumberActor.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Blueprint/UserWidget.h"

ADamageNumberActor::ADamageNumberActor()
{
	PrimaryActorTick.bCanEverTick = true;

	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	RootComponent = WidgetComp;
	// 화면 공간: 항상 카메라를 향하고, 거리와 무관하게 크기가 같고, 지형에 가려지지 않는다
	WidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComp->SetDrawAtDesiredSize(true);
	WidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADamageNumberActor::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSeconds);   // 시간 지나면 자동 Destroy
	SpawnTime = GetWorld()->GetTimeSeconds();

	WidgetComp->SetWidgetSpace(bScreenSpace ? EWidgetSpace::Screen : EWidgetSpace::World);

	if (bArcMotion)
	{
		// 좌우는 카메라 기준 — 어느 방향에서 봐도 화면에서 옆으로 튄다
		FVector SideDir = FVector::RightVector;
		if (const APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			SideDir = FRotationMatrix(Cam->GetCameraRotation()).GetUnitAxis(EAxis::Y);
		}
		const float Sign = FMath::RandBool() ? 1.f : -1.f;
		const float Vary = 1.f + FMath::FRandRange(-ArcSpeedVariance, ArcSpeedVariance);

		ArcVelocity = SideDir * (ArcSideSpeed * Sign * Vary) + FVector(0.f, 0.f, ArcUpSpeed * Vary);
	}
}

void ADamageNumberActor::InitDamage(float Amount)
{
	DamageAmount = Amount;
	OnInitDamage(Amount);
}

void ADamageNumberActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bArcMotion)
	{
		ArcVelocity.Z += ArcGravity * DeltaTime;         // 올라갔다가 중력에 눌려 떨어진다
		AddActorWorldOffset(ArcVelocity * DeltaTime);
	}
	else if (RiseSpeed != 0.f)
	{
		AddActorWorldOffset(FVector(0.f, 0.f, RiseSpeed * DeltaTime));
	}

	// 수명 끝으로 갈수록 서서히 사라진다. 글자 색이 아니라 위젯 투명도를 낮춰야
	// 외곽선까지 같이 사라진다 (색 알파만 낮추면 검은 테두리만 남는다)
	if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
	{
		const float Elapsed = GetWorld()->TimeSince(SpawnTime);
		const float FadeStart = LifeSeconds * FMath::Clamp(FadeStartRatio, 0.f, 1.f);
		const float FadeLength = FMath::Max(LifeSeconds - FadeStart, KINDA_SMALL_NUMBER);
		Widget->SetRenderOpacity(1.f - FMath::Clamp((Elapsed - FadeStart) / FadeLength, 0.f, 1.f));
	}

	if (bFaceCamera && !bScreenSpace)
	{
		if (APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			const FVector ToCam = Cam->GetCameraLocation() - GetActorLocation();
			// 위젯 컴포넌트의 정면(+X)이 카메라를 향하도록 Yaw 정렬 (FindLookAtRotation과 동일).
			// Yaw만 맞춰 텍스트는 수평 유지.
			SetActorRotation(FRotator(0.f, ToCam.Rotation().Yaw, 0.f));
		}
	}
}
