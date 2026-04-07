// Copyright Epic Games, Inc. All Rights Reserved.
#include "HelicopterEnemy.h"

AHelicopterEnemy::AHelicopterEnemy()
{
    AttackDamage         = 25.f;
    AttackRange          = 2500.f;
    AttackCooldown       = 0.3f;
    PreferredCombatRange = 2000.f;
    HoverHeight          = 800.f;
    FlySpeed             = 700.f;
    AlertRadius          = 3000.f;
}
