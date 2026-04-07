// Copyright Epic Games, Inc. All Rights Reserved.
#include "MGEnemy.h"

AMGEnemy::AMGEnemy()
{
    AttackDamage         = 8.f;
    AttackRange          = 1800.f;
    AttackCooldown       = 0.08f;
    PreferredCombatRange = 1500.f;
    PatrolRadius         = 0.f;
    bHoldPosition        = true;
}
