// Copyright Epic Games, Inc. All Rights Reserved.

#include "DamageNumberActor.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

ADamageNumberActor::ADamageNumberActor()
{
	PrimaryActorTick.bCanEverTick = true;

	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	RootComponent = WidgetComp;
	WidgetComp->SetWidgetSpace(EWidgetSpace::World);   // World 스페이스라 거리에 따라 원근으로 작아짐
	WidgetComp->SetDrawAtDesiredSize(true);
	WidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADamageNumberActor::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSeconds);   // 시간 지나면 자동 Destroy
}

void ADamageNumberActor::InitDamage(float Amount)
{
	DamageAmount = Amount;
	OnInitDamage(Amount);
}

void ADamageNumberActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RiseSpeed != 0.f)
	{
		AddActorWorldOffset(FVector(0.f, 0.f, RiseSpeed * DeltaTime));
	}

	if (bFaceCamera)
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
