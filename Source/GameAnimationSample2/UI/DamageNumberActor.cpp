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
			// Yaw만 맞춰 텍스트를 수평으로 유지. 글자가 뒤집혀 보이면 +180.f를 제거.
			SetActorRotation(FRotator(0.f, ToCam.Rotation().Yaw + 180.f, 0.f));
		}
	}
}
