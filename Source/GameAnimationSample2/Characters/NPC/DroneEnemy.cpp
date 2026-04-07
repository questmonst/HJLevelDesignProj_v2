// Copyright Epic Games, Inc. All Rights Reserved.
#include "DroneEnemy.h"

ADroneEnemy::ADroneEnemy()
{
    AttackDamage         = 10.f;
    AttackRange          = 800.f;
    AttackCooldown       = 0.5f;
    PreferredCombatRange = 500.f;
    HoverHeight          = 300.f;
    FlySpeed             = 500.f;
}
