// Copyright Epic Games, Inc. All Rights Reserved.
#include "SmallTurret.h"

ASmallTurret::ASmallTurret()
{
    MaxHealth      = 1.f;   // 미카 펀치 한 방에 파괴
    AttackDamage   = 10.f;
    AttackRange    = 1200.f;
    AttackCooldown = 0.4f;
    RotationSpeed  = 120.f;
}
