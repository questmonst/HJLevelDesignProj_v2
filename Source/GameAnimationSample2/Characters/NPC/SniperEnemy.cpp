// Copyright Epic Games, Inc. All Rights Reserved.
#include "SniperEnemy.h"

ASniperEnemy::ASniperEnemy()
{
    AttackDamage         = 60.f;
    AttackRange          = 3500.f;
    AttackCooldown       = 3.0f;
    PreferredCombatRange = 3000.f;
    PatrolRadius         = 0.f;
    bHoldPosition        = true;
}
