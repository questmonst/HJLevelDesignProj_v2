// Copyright Epic Games, Inc. All Rights Reserved.
#include "ShotgunEnemy.h"

AShotgunEnemy::AShotgunEnemy()
{
    AttackDamage         = 40.f;
    AttackRange          = 600.f;
    AttackCooldown       = 1.4f;
    PreferredCombatRange = 500.f;
    PatrolRadius         = 400.f;
    ChargeRange          = 300.f;
    ChargeAfterEngageTime = 5.f;
}
