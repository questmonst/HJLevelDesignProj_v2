// Copyright Epic Games, Inc. All Rights Reserved.
#include "ShieldEnemy.h"
#include "Kismet/KismetMathLibrary.h"

AShieldEnemy::AShieldEnemy()
{
    AttackDamage         = 20.f;
    AttackRange          = 500.f;
    AttackCooldown       = 1.2f;
    PreferredCombatRange = 300.f;
    PatrolRadius         = 300.f;
    ShieldMaxHealth      = 150.f;
    ShieldCurrentHealth  = ShieldMaxHealth;
}

float AShieldEnemy::GetShieldPercent() const
{
    return ShieldMaxHealth > 0.f ? ShieldCurrentHealth / ShieldMaxHealth : 0.f;
}

float AShieldEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    // 쉴드가 활성화된 경우, 전방 ShieldBlockAngle 이내 공격을 쉴드가 흡수
    if (bShieldActive && DamageCauser)
    {
        FVector ToDamager = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        float DotAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GetActorForwardVector(), ToDamager)));

        if (DotAngle <= ShieldBlockAngle)
        {
            ShieldCurrentHealth -= DamageAmount;
            if (ShieldCurrentHealth <= 0.f)
            {
                ShieldCurrentHealth = 0.f;
                bShieldActive       = false;
                OnShieldBreak();
            }
            return DamageAmount;
        }
    }

    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AShieldEnemy::OnShieldBreak_Implementation()
{
    // BP에서 이펙트/사운드 처리
}
