# BP 애니메이션 셋업 레퍼런스

> C++ 로직이 구현되어 있고, BP에서 몽타주 할당 + AnimBP 변수 연결만 하면 되는 항목들을 정리한 문서.

---

## 목차

1. [클래스 구조](#클래스-구조)
2. [트래버설 시스템](#트래버설-시스템)
3. [미카 펀치 / 대시 / 랜딩](#미카-펀치--대시--랜딩)
4. [기본 TPS (무기 교체 / 조준 / 수류탄)](#기본-tps)
5. [낙하 애니메이션](#낙하-애니메이션)
6. [미유 지원사격](#미유-지원사격)
7. [중력 방향 변경](#중력-방향-변경)
8. [낙사 리셋 시스템](#낙사-리셋-시스템)
9. [파괴 가능 엄폐물](#파괴-가능-엄폐물)
10. [스플라인 트랙 액터](#스플라인-트랙-액터)
11. [전체 BP 변수 참조표](#전체-bp-변수-참조표)

---

## 클래스 구조

```
ACharacter
└── ACharacterBase          (체력/팀/사망)
    ├── APlayerCharacter    (카메라/입력/무기/수류탄/트래버설)
    │   └── AMikaCharacter  (펀치/대시/랜딩)
    └── AEnemyCharacter
        └── ABossCharacter (아키라)
```

---

## 트래버설 시스템

### 관련 파일
- `Source/.../Characters/Player/TraversalComponent.h/.cpp`
- `PlayerCharacter`에 `UTraversalComponent`, `UMotionWarpingComponent` 포함

### 동작 흐름

```
Jump 키 입력
  └─ APlayerCharacter::Jump()
       ├─ TraversalComponent->TryTraversal()
       │     ├─ 전방 구체 스윕 → 장애물 벽면 감지
       │     ├─ 위→아래 라인 트레이스 → 상단 높이 계산
       │     ├─ 착지 공간 캡슐 스윕 → 헤드룸 확인
       │     ├─ MotionWarpingComponent에 워프 타겟 등록
       │     │     "TraversalLedge"   = 가장자리 위치
       │     │     "TraversalLanding" = 착지 위치
       │     └─ OnTraversalStart 이벤트 발동
       │
       └─ 감지 실패 시 → Super::Jump() (일반 점프)
```

### 트래버설 타입 기준 (높이 = 발 기준 상대 높이)

| ETraversalType | 높이 범위 | 기본 애니메이션 |
|---|---|---|
| Hurdle | < 60 cm | `M_Neutral_Traversal_Hurdle_1_0_run_F_Rfoot` |
| Vault  | 60 ~ 150 cm | `M_Neutral_Traversal_Vault_1_0_run_F_Lfoot` |
| Mantle | 150 ~ 230 cm | `M_Neutral_Traversal_Mantle_1_0_run_F_Rfoot` |
| Climb  | 230 ~ 285 cm | `M_Neutral_Traversal_Climb_Start_2_5_run_F_Rfoot` |

> 애니메이션 경로: `Content/Characters/UEFN_Mannequin/Animations/Traversal/[Type]/`

### BP에서 해야 할 작업

#### 1. BP_MikaCharacter — `OnTraversalStart` 이벤트 오버라이드

```
Event OnTraversalStart (Type, WarpTarget, Ledge)
  └─ Switch on ETraversalType
       ├─ Hurdle  → PlayAnimMontage(몽타주_Hurdle)
       ├─ Vault   → PlayAnimMontage(몽타주_Vault)
       ├─ Mantle  → PlayAnimMontage(몽타주_Mantle)
       └─ Climb   → PlayAnimMontage(몽타주_Climb)
```

- 루트모션이 켜진 몽타주 사용 필요 (`Enable Root Motion = true`)
- MotionWarping이 활성화된 경우 몽타주 안에서 `TraversalLedge` / `TraversalLanding` 워프 윈도우 설정

#### 2. 각 몽타주 — AnimNotify 추가

몽타주 끝 지점에 AnimNotify 추가:

```
AnimNotify 이름: TraversalFinish
  → Event Graph: AnimNotify_TraversalFinish
       └─ GetComponentByClass(TraversalComponent)
            └─ TraversalComponent->FinishTraversal()
```

> `FinishTraversal()`이 호출되면:
> - 이동 모드 MOVE_Walking 복원
> - 착지 위치로 스냅
> - `bIsTraversing = false`

#### 3. (옵션) MotionWarping 워프 윈도우 설정

몽타주 트랙에 `MotionWarping` 트랙 추가 → 워프 윈도우 구간 설정:
- **손이 가장자리에 닿는 구간**: Warp Target Name = `TraversalLedge`
- **착지 구간**: Warp Target Name = `TraversalLanding`

이렇게 하면 장애물 높이가 약간 달라도 애니메이션이 자동으로 맞춰짐.

### AnimBP에서 읽을 수 있는 변수

```
TraversalComponent->IsTraversing()       → bool  (트래버설 중 여부)
TraversalComponent->GetTraversalType()   → ETraversalType
TraversalComponent->GetWarpTarget()      → FTransform
TraversalComponent->GetLedgeTransform()  → FTransform
```

### UPROPERTY (에디터에서 조정 가능)

| 변수 | 기본값 | 설명 |
|---|---|---|
| ForwardTraceDistance | 80 cm | 전방 장애물 감지 거리 |
| HurdleMaxHeight | 60 cm | 허들 최대 높이 |
| VaultMaxHeight | 150 cm | 볼트 최대 높이 |
| MantleMaxHeight | 230 cm | 맨틀 최대 높이 |
| ClimbMaxHeight | 285 cm | 클라임 최대 높이 (이상은 감지 안 함) |
| MinObstacleHeight | 20 cm | 감지 최소 높이 |
| LandingForwardOffset | 40 cm | 착지 위치 앞 오프셋 |

---

## 미카 펀치 / 대시 / 랜딩

### 관련 파일
- `Source/.../Characters/Player/MikaCharacter.h/.cpp`

### 동작 흐름

```
좌클릭(Shoot) 누름
  ├─ bIsAiming == true  → 무기 발사 (Super::StartFire)
  └─ bIsAiming == false → 미카 펀치 충전 시작
       ├─ bIsChargingPunch = true
       ├─ 중력 스케일 낮춤 (ChargeGravityScale)
       ├─ 이동속도 낮춤 (ChargeMovementScale × WalkSpeed)
       ├─ 지상/공중 제동력 강화
       ├─ ForcedMaxChargeTime 후 자동 발동
       └─ Tick: 캐릭터가 카메라 Yaw 방향을 매 프레임 추적

좌클릭 뗌
  ├─ bIsAiming == true  → 무기 사격 중지
  └─ bIsChargingPunch
       ├─ HeldTime < MinChargeTime → 취소 (아무것도 안 함)
       ├─ CanTriggerLanding() == true → 미카 랜딩 시작
       │     조건: 공중 + 조준 중 + 카메라 pitch < -LandingDownAngle
       │           + 발 아래 LandingMinHeight 이상 공간
       │     → LaunchCharacter(아래로, LandingDiveSpeed)
       │     → Landed() 오버라이드에서 ApplyRadialDamage 발동
       └─ 일반 대시
             ChargeRatio = (HeldTime - MinChargeTime) / (MaxChargeTime - MinChargeTime)
             → StartDash(ChargeRatio)
                  → 카메라 방향으로 LaunchCharacter (공중 포함)
                  → PunchHitbox 활성화
                  → DashDuration 후 EndDash()
                  → 히트박스 비활성화 + 쿨타임 시작
```

### 펀치 쿨타임
- 미카 펀치(대시)와 미카 랜딩이 **동일한 쿨타임(`PunchCooldown`)** 공유
- 쿨타임 중 `bCanPunch = false` → 펀치/랜딩 모두 불가

### BP에서 해야 할 작업 (AnimBP)

| 상태 변수 | 읽는 방법 | 용도 |
|---|---|---|
| 충전 중 | `bIsChargingPunch` (C++ protected) | AnimBP에서 충전 포즈 블렌딩 |
| 대시 중 | `bIsDashing` | 대시 애니메이션 트리거 |
| 다이브 랜딩 중 | `bIsDivingLanding` | 낙하 랜딩 애니메이션 트리거 |

> `bIsChargingPunch`, `bIsDashing`, `bIsDivingLanding` 은 C++ `protected`이므로  
> AnimBP에서 읽으려면 **BlueprintReadOnly를 추가하거나 Getter BlueprintPure 함수를 추가**해야 함.  
> 현재 AnimBP에서 직접 접근이 필요하면 요청 시 getter 추가 가능.

### 카메라 상태머신 (Tick에서 매 프레임 보간)

```
bIsAiming        → TargetArm = NormalSpringArmLength, TargetFOV = AimFOV
bIsDashing       → TargetArm = DashSpringArmLength,   TargetFOV = DashFOV
bIsChargingPunch → TargetArm = ChargeSpringArmLength, TargetFOV = ChargeFOV
기본             → TargetArm = NormalSpringArmLength, TargetFOV = NormalFOV
```

### UPROPERTY (에디터에서 조정 가능)

| 카테고리 | 변수 | 기본값 | 설명 |
|---|---|---|---|
| Punch | PunchDamage | 30 | 대시 히트 데미지 |
| Punch | PunchCooldown | 1.0 s | 펀치/랜딩 공유 쿨타임 |
| Punch | ChargeGravityScale | 0.2 | 충전 중 중력 배율 |
| Punch | ChargeMovementScale | 0.3 | 충전 중 이동속도 배율 |
| Punch | ChargeBrakingDeceleration | 600 | 충전 중 지상 제동력 |
| Punch | ChargeAirBrakingDeceleration | 1000 | 충전 중 공중 제동력 |
| Punch | MinChargeTime | 0.1 s | 최소 충전 시간 (미만 시 취소) |
| Punch | MaxChargeTime | 1.5 s | 속도가 최대가 되는 충전 시간 |
| Punch | ForcedMaxChargeTime | 2.0 s | 이 이상 충전 시 강제 발동 |
| Punch | MinDashSpeed | 800 | 최소 충전 시 대시 속도 |
| Punch | MaxDashSpeed | 2400 | 최대 충전 시 대시 속도 |
| Punch | DashDuration | 0.25 s | 대시 지속 시간 |
| Punch | DashBrakingDeceleration | 800 | 대시 중 공중 제동력 |
| Camera | ChargeSpringArmLength | 180 | 충전 중 카메라 거리 |
| Camera | ChargeFOV | 80 | 충전 중 FOV |
| Camera | DashSpringArmLength | 420 | 대시 중 카메라 거리 |
| Camera | DashFOV | 105 | 대시 중 FOV |
| Camera | CameraInterpSpeed | 10 | 카메라 보간 속도 |
| Landing | LandingDamage | 60 | 랜딩 방사형 데미지 |
| Landing | LandingRadius | 300 | 랜딩 데미지 반경 |
| Landing | LandingDownAngle | 60 ° | 랜딩 발동 최소 아래 각도 |
| Landing | LandingMinHeight | 300 cm | 랜딩 발동 최소 높이 |
| Landing | LandingDiveSpeed | 2000 | 다이브 낙하 속도 |

---

## 기본 TPS

### 관련 파일
- `Source/.../Characters/Player/PlayerCharacter.h/.cpp`

### 조준 (Cover Peek 포함)

```
우클릭 누름 → StartAim()
  bIsAiming = true
  FOV = AimFOV
  bUseControllerRotationYaw = true
  bOrientRotationToMovement = false
  Tick → 좌/우 엄폐물 라인 트레이스
         왼쪽 엄폐 → 카메라 오른쪽 오프셋 +CoverPeekOffset
         오른쪽 엄폐 → 카메라 왼쪽 오프셋 -CoverPeekOffset

우클릭 뗌 → StopAim()
  FOV = NormalFOV
  bOrientRotationToMovement = true 복원
```

### 무기 교체

```
숫자키 1/2/3 → EquipWeapon(index)
  ├─ bIsSwapping 중이면 무시
  ├─ 현재 무기 → WeaponHolsterSocket으로 이동
  ├─ WeaponSwapDelay 후 FinishEquipWeapon() 호출
  └─ 새 무기 → WeaponAttachSocket으로 이동
```

### 수류탄 궤도

```
Throw 키 누름 → StartGrenadeThrow()
  bIsPreparingThrow = true
  Tick → UpdateTrajectory()
         SplineComponent + SplineMeshPool(최대 20개)로 궤도 시각화

Throw 키 뗌 → ReleaseGrenadeThrow()
  GrenadeBase 스폰 + Launch(ViewDirection × GrenadeThrowSpeed)
  궤도 클리어
```

### UPROPERTY (에디터에서 조정 가능)

| 카테고리 | 변수 | 기본값 | 설명 |
|---|---|---|---|
| Movement | WalkSpeed | 400 | 걷기 속도 |
| Movement | SprintSpeed | 700 | 달리기 속도 |
| Camera | NormalFOV | 90 | 기본 FOV |
| Camera | AimFOV | 60 | 조준 FOV |
| Camera | CoverTraceDistance | 80 cm | 엄폐물 감지 거리 |
| Camera | CoverPeekOffset | 80 cm | 엄폐 시 카메라 오프셋 |
| Camera | CoverPeekInterpSpeed | 8 | 엄폐 카메라 보간 속도 |
| Camera | DefaultSocketOffsetY | 60 cm | 기본 카메라 우측 오프셋 |
| Weapon | WeaponAttachSocket | `hand_r` | 장착 소켓 |
| Weapon | WeaponHolsterSocket | (비어있음) | 수납 소켓 (비어있으면 숨김) |
| Weapon | WeaponSwapDelay | 0.3 s | 무기 교체 딜레이 |
| Weapon | MaxWeaponSlots | 3 | 최대 무기 슬롯 |
| Grenade | GrenadeThrowSpeed | 1200 | 수류탄 투척 속도 |
| Grenade | TrajectoryMeshScale | 0.1 | 궤도 메시 스케일 |

---

## 낙하 애니메이션

### 관련 파일
- `Source/.../Characters/Player/PlayerCharacter.h/.cpp`

### 동작 흐름

```
매 Tick (PlayerCharacter::Tick)
  └─ bIsFalling     = CharacterMovement->IsFalling()
     CurrentFallSpeed = (ZVel < 0) ? -ZVel : 0   ← 하강 중만 양수

착지 시 (APlayerCharacter::Landed)
  └─ LandingSpeed = Tick에서 추적하던 CurrentFallSpeed
       ├─ >= HardLandingSpeedThreshold → bHardLanding = true
       └─ < 임계값                     → bHardLanding = false
     → OnLanding(bHardLanding) 이벤트 발동
     → bIsHardLanding = bHardLanding
```

### AnimBP에서 읽을 변수

| 변수 | 타입 | 용도 |
|---|---|---|
| `bIsFalling` | bool | 낙하/점프 상승 중 여부 — 낙하 포즈 전환 |
| `CurrentFallSpeed` | float | 낙하 속도(cm/s, 양수) — 빠른/느린 낙하 블렌딩 |
| `bIsHardLanding` | bool | 하드 랜딩 직후 — 하드 랜딩 몽타주 트리거 |

### BP에서 해야 할 작업

#### 1. AnimBP — State Machine 연결

```
[Idle/Walk/Run]
    ↓ bIsFalling == true
  [Fall_Start]  → 점프 상승 or 낙하 시작 포즈
    ↓ CurrentFallSpeed > 0
  [Falling]     → CurrentFallSpeed로 느림/빠름 블렌딩
    ↓ bIsFalling == false (착지)
  [Land]        → OnLanding 이벤트로 몽타주 재생
    ↓ 몽타주 끝 Notify: ResetHardLanding()
  [Idle/Walk/Run]
```

#### 2. BP_PlayerCharacter — `OnLanding` 이벤트 오버라이드

```
Event OnLanding (bHardLanding)
  ├─ bHardLanding == true  → PlayAnimMontage(HardLandMontage)
  └─ bHardLanding == false → PlayAnimMontage(SoftLandMontage)
```

#### 3. 각 몽타주 끝 — AnimNotify 추가

```
AnimNotify: LandingFinish
  → BP에서: GetPlayerCharacter → ResetHardLanding() 호출
```

> `ResetHardLanding()`을 호출하지 않으면 `bIsHardLanding`이 계속 true로 남음.

### UPROPERTY (에디터에서 조정 가능)

| 변수 | 기본값 | 설명 |
|---|---|---|
| `HardLandingSpeedThreshold` | 600 cm/s | 이 속도 이상이면 하드 랜딩 판정 |

---

## 미유 지원사격

### 관련 파일
- `Source/.../Characters/NPC/MiyuCharacter.h/.cpp`

### 동작 흐름

```
레벨 BP / GameMode에서:
  MiyuCharacter->SetTarget(AkiraRef)
    └─ OnTargetAcquired 이벤트 발동 (BP에서 경계 애니메이션 등)
    └─ 타이머 시작 (FirstFireDelay 후 첫 체크, 이후 FireCooldown 주기)

매 FireCooldown마다 TryFire() 실행:
  └─ CanFireAtTarget() 체크
       ├─ 거리 ≤ SupportFireRange?
       ├─ Miyu → Target LineTrace 가 지오메트리에 막히지 않음?
       └─ Player가 Miyu와 Target 사이에 없음?
            ↓ 모두 통과
       OnSupportFire(Target) 이벤트 발동

미유 못 쏘는 상황 (플레이어가 사격각 막고 있을 때):
  → 플레이어가 Miyu-Target 라인에서 비켜주면 자동으로 다음 체크 때 발사
```

### 핵심 메커니즘 — 사격각 확보

```
Miyu ──[LineTrace]──> [Player?] ──> Target
                          ↑
               플레이어가 여기 있으면 차단 → 발사 안 함
               플레이어가 비켜주면 → 발사 가능
```

> `HasLineOfSightToTarget`이 `this`와 `Target`만 무시하고 Player는 무시 목록에 없으므로,  
> 플레이어가 라인 위에 있으면 자동으로 차단됨.

### BP에서 해야 할 작업

#### 1. 레벨 BP / GameMode — 타겟 설정

```
Event BeginPlay (or Phase 시작 트리거)
  └─ Get MiyuCharacter Reference
       └─ SetTarget(AkiraCharacterRef)

아키라 사망 or 페이즈 전환 시:
  └─ MiyuCharacter->ClearTarget()
```

#### 2. BP_MiyuCharacter — `OnSupportFire` 이벤트 오버라이드

```
Event OnSupportFire (Target)
  ├─ PlayAnimMontage(SupportFireMontage)
  └─ 발사체 or 히트스캔 처리
       예) SpawnActor(ProjectileClass, MuzzleSocket)
           → Projectile->Launch(Target->GetActorLocation())
```

#### 3. BP_MiyuCharacter — `OnTargetAcquired` / `OnTargetLost` 오버라이드

```
Event OnTargetAcquired (Target)
  └─ 경계 포즈 전환 애니메이션
  └─ 타겟 방향으로 회전 (SetActorRotation or AI Controller)

Event OnTargetLost
  └─ 대기 포즈 복귀
```

### AnimBP에서 읽을 변수

| 변수 | 타입 | 용도 |
|---|---|---|
| `bIsFiring` | bool | 사격 타이머 활성화 중 여부 — 경계 포즈 유지 |
| `CurrentTarget` | AActor* | 현재 타겟 (유효성 체크) |

### UPROPERTY (에디터에서 조정 가능)

| 변수 | 기본값 | 설명 |
|---|---|---|
| `SupportFireRange` | 1500 cm | 최대 사격 거리 |
| `FireCooldown` | 2.0 s | 발사 간격 |
| `FirstFireDelay` | 0.5 s | SetTarget 후 첫 체크까지 대기 시간 |

---

## 중력 방향 변경

### 관련 파일
- `Source/.../Characters/Player/PlayerCharacter.h/.cpp`

### 레벨 블루프린트에서 호출 방법

```
Get Player Character
  └─ Cast to APlayerCharacter
       ├─ SetGravityDirection(NewDirection)   ← 중력 방향 변경
       └─ ResetGravity()                      ← 기본값 (0,0,-1) 복원
```

### 방향 벡터 예시

| 호출 | 효과 |
|---|---|
| `SetGravityDirection(0, 0, -1)` | 기본 (바닥이 아래) |
| `SetGravityDirection(0, 0, 1)` | 천장이 바닥 |
| `SetGravityDirection(1, 0, 0)` | 오른쪽 벽이 바닥 |
| `SetGravityDirection(-1, 0, 0)` | 왼쪽 벽이 바닥 |
| `ResetGravity()` | 기본값 복원 |

> 벡터 정규화는 내부에서 자동 처리. 임의 방향도 가능.

### CLAUDE.md 기획 기준 사용 흐름

```
트리거 볼륨 진입 or Sequencer 이벤트 트랙
  └─ Level BP: SetGravityDirection(0, 0, 1)  ← 중력 전환
       ├─ 캐릭터 이동/점프/착지 전부 새 방향 기준으로 자동 재계산
       └─ 카메라 회전은 Sequencer로 별도 연출

구간 종료
  └─ Level BP: ResetGravity()
```

### 주의사항
- 중력 변경은 **해당 캐릭터에만** 적용됨 (다른 물리 오브젝트는 영향 없음)
- 다른 물리 액터도 같이 바꾸려면 레벨의 `WorldSettings → Default Gravity Z` 또는 각 액터의 `GravityScale` 조정 필요

---

## 낙사 리셋 시스템

### 관련 파일
- `Source/.../Environment/FallResetTrigger.h/.cpp` (Actor A — 콜리전)
- `Source/.../Environment/FallResetTarget.h/.cpp` (Actor B — 목표 위치)

### 레벨 배치 방법

1. `FallResetTrigger` 를 낙사 구역 아래에 배치
2. `FallResetTarget` 을 리스폰할 위치에 배치 (초록 화살표가 캐릭터가 바라볼 방향)
3. `FallResetTrigger` Detail 패널 → `Reset Target` 칸에 `FallResetTarget` 액터 연결

### 동작 흐름

```
플레이어가 FallResetTrigger 콜리전에 닿음
  └─ 카메라 페이드 아웃 (FadeOutDuration)
       └─ 페이드 완료 후
            ├─ FallResetTarget 위치/방향으로 SetActorTransform
            ├─ 속도 초기화 (bResetVelocity = true 시)
            └─ 카메라 페이드 인 (FadeInDuration)
```

- AI 캐릭터는 무시 (PlayerController가 있는 캐릭터만 반응)
- 텔레포트 중 중복 발동 방지 (`bIsTeleporting` 플래그)

### UPROPERTY (에디터에서 조정 가능)

| 변수 | 기본값 | 설명 |
|---|---|---|
| ResetTarget | — | 이동할 FallResetTarget 액터 |
| FadeOutDuration | 0.2 s | 텔레포트 전 페이드 아웃 시간 (0 = 즉시) |
| FadeInDuration | 0.3 s | 텔레포트 후 페이드 인 시간 (0 = 즉시) |
| bResetVelocity | true | 텔레포트 시 속도 초기화 여부 |
| TriggerBox Extent | 200×200×50 | 트리거 박스 크기 (에디터에서 조정) |

---

## 파괴 가능 엄폐물

### 관련 파일
- `Source/.../Environment/DestructibleCover.h/.cpp`

### 동작 흐름

```
TakeDamage() 호출
  └─ CurrentHealth -= Damage
       └─ CurrentHealth <= 0
            └─ OnCoverDestroyed() 호출 (BlueprintNativeEvent)
                 → BP에서 Chaos Geometry Collection 스폰 or 파괴 이펙트 재생
```

### BP에서 해야 할 작업

`OnCoverDestroyed` 이벤트 오버라이드:
```
Event OnCoverDestroyed
  ├─ SpawnActor(GeometryCollection, GetActorTransform)  ← Chaos 파편
  ├─ SpawnEmitter(파괴 이펙트)
  ├─ PlaySound(파괴 사운드)
  └─ DestroyActor(Self)
```

### UPROPERTY

| 변수 | 기본값 | 설명 |
|---|---|---|
| MaxHealth | — | 최대 체력 |
| CurrentHealth | — | 현재 체력 |
| GetHealthPercent() | — | 0~1 체력 비율 (BlueprintPure) |

---

## 스플라인 트랙 액터

### 관련 파일
- `Source/.../Environment/SplineTrackActor.h/.cpp`

### 사용 방법

1. 레벨에 `ASplineTrackActor` 배치
2. 에디터에서 스플라인 포인트 편집
3. Detail 패널에서 `TrackMesh`, `MeshMaterial` 할당
4. **`RebuildTrack` 버튼 클릭** → 스플라인을 따라 메시 생성
   - `OnConstruction`에서는 자동 생성 안 함 (의도적)
   - 매번 RebuildTrack 눌러야 갱신됨

### UPROPERTY

| 변수 | 기본값 | 설명 |
|---|---|---|
| TrackMesh | — | 반복 배치할 스태틱 메시 |
| MeshMaterial | — | 메시 머티리얼 |
| MeshSpacing | — | 메시 간격 (cm) |
| ForwardAxis | — | 메시 전진 방향 축 |
| bEnableCollision | — | 충돌 활성화 여부 |
| RollOffsetDegrees | — | 메시 롤 오프셋 |
| bCloseLoop | — | 루프 연결 세그먼트 추가 여부 |

---

## 적 시스템

### 클래스 구조

```
AEnemyCharacter (베이스 — AI + 무기 + 전투 로직)
├── AAREnemy           (압박형)
├── AShotgunEnemy      (기회주의형)
├── ASniperEnemy       (고지대 위협형)
├── AMGEnemy           (개활지 방어형)
├── AShieldEnemy       (전선 형성형 — 쉴드 시스템 포함)
└── ALargeSweeperEnemy (공간 지배형)

ABaseAirUnit (공중 유닛 베이스 — Flying 모드)
├── ADroneEnemy
└── AHelicopterEnemy

ABaseTurret (고정 포탑 베이스 — AActor)
└── ASmallTurret

AEnemyAIController (AI 컨트롤러 — Perception + BT 실행)
```

### AI 컨트롤러 동작

```
OnPossess
  └─ AEnemyCharacter->GetBehaviorTree() → RunBehaviorTree()
       └─ Blackboard PatrolOrigin = 스폰 위치

OnTargetPerceptionUpdated
  ├─ 감지 성공 → BB: TargetActor, TargetLocation, bCanSeeTarget=true, bIsAlerted=true
  │               → EnemyCharacter->AlertEnemy() (반경 내 아군에게도 전파)
  └─ 감지 실패 → BB: bCanSeeTarget=false → EnemyCharacter->StopFiring()
```

### Blackboard 키 이름 (BT 에셋 제작 시 이 이름으로 생성)

| 키 이름 | 타입 | 설명 |
|---|---|---|
| `TargetActor` | Object | 추적 대상 액터 |
| `TargetLocation` | Vector | 마지막 목격 위치 |
| `bCanSeeTarget` | Bool | 현재 시야 내 여부 |
| `bIsAlerted` | Bool | 경계 상태 여부 |
| `PatrolOrigin` | Vector | 순찰 원점 (스폰 위치) |

### BP에서 해야 할 작업

#### 1. BehaviorTree 에셋 제작 (에디터)

각 적 타입별 BB + BT 에셋을 만들고, BP_[EnemyType] Detail 패널의 `BehaviorTree` 칸에 연결.

**공통 BT 노드가 읽는 C++ 변수들:**
- `GetPreferredCombatRange()` — 유지할 전투 거리
- `GetAttackRange()` — 사격 가능 거리
- `GetIsAlerted()` — 경계 상태
- `AShotgunEnemy::ChargeRange` — 돌격 전환 거리
- `AShotgunEnemy::ChargeAfterEngageTime` — 돌격 전환 시간
- `ASniperEnemy::bHoldPosition` — 위치 고수 여부
- `AMGEnemy::bHoldPosition` — 위치 고수 여부
- `ALargeSweeperEnemy::MeleeSwipeRadius` — 근접 스윕 범위

**BT에서 호출하는 C++ 함수:**
- `EnemyCharacter->FireAtTarget()` — 사격 시작
- `EnemyCharacter->StopFiring()` — 사격 중지

#### 2. AShieldEnemy — 쉴드 파괴 이벤트

```
Event OnShieldBreak
  ├─ SpawnEmitter (쉴드 파괴 이펙트)
  ├─ PlaySound (쉴드 파괴 사운드)
  └─ 이후 일반 데미지 정상 적용
```

- `IsShieldActive()` — AnimBP에서 쉴드 방어 포즈 여부 판단에 사용
- `GetShieldPercent()` — UI 쉴드 게이지

#### 3. ABaseTurret — 파괴 이벤트

```
Event OnTurretDestroyed
  ├─ SpawnActor (GeometryCollection) — Chaos 파괴
  └─ DestroyActor(Self)
```

- `MaxHealth = 1` (SmallTurret) → 미카 펀치 한 방에 파괴

### AI 컨트롤러 UPROPERTY (에디터에서 조정)

| 변수 | 기본값 | 설명 |
|---|---|---|
| SightRadius | 2000 cm | 시야 반경 |
| LoseSightRadius | 2500 cm | 시야 상실 반경 |
| PeripheralVisionAngleDeg | 60 ° | 주변 시야각 |
| HearingRange | 1200 cm | 청각 반경 |

### 적 타입별 기본 수치

| 클래스 | Damage | Range | Cooldown | PreferredRange |
|---|---|---|---|---|
| AAREnemy | 15 | 1500 | 0.7s | 900 |
| AShotgunEnemy | 40 | 600 | 1.4s | 500 |
| ASniperEnemy | 60 | 3500 | 3.0s | 3000 |
| AMGEnemy | 8 | 1800 | 0.08s | 1500 |
| AShieldEnemy | 20 | 500 | 1.2s | 300 |
| ALargeSweeperEnemy | 45 | 350 | 1.8s | 200 |
| ADroneEnemy | 10 | 800 | 0.5s | 500 |
| AHelicopterEnemy | 25 | 2500 | 0.3s | 2000 |
| ASmallTurret | 10 | 1200 | 0.4s | — |

---

## 전체 BP 변수 참조표

### AnimBP에서 읽어야 할 상태 변수

| 클래스 | 변수/함수 | 타입 | 용도 |
|---|---|---|---|
| APlayerCharacter | `bIsAiming` | bool | 조준 상태 (Aim 포즈) |
| AMikaCharacter | `bIsChargingPunch` | bool | 충전 중 포즈 |
| AMikaCharacter | `bIsDashing` | bool | 대시 애니메이션 |
| AMikaCharacter | `bIsDivingLanding` | bool | 낙하 랜딩 애니메이션 |
| UTraversalComponent | `IsTraversing()` | bool | 트래버설 진행 중 |
| UTraversalComponent | `GetTraversalType()` | ETraversalType | 트래버설 타입 |

### BP에서 호출하는 함수 (AnimNotify 등에서)

| 함수 | 위치 | 호출 시점 |
|---|---|---|
| `TraversalComponent->FinishTraversal()` | UTraversalComponent | 트래버살 몽타주 끝 AnimNotify |
| `OnCoverDestroyed` | ADestructibleCover | 파괴 연출 BP에서 오버라이드 |

### BP에서 설정해야 할 에셋

| 변수 | 설정 위치 | 설명 |
|---|---|---|
| `DefaultWeaponClass` | BP_MikaCharacter Details | 기본 무기 클래스 |
| `GrenadeClass` | BP_MikaCharacter Details | 수류탄 클래스 |
| `TrajectoryMesh` | BP_MikaCharacter Details | 궤도 표시용 메시 |
| `TrajectoryMaterial` | BP_MikaCharacter Details | 궤도 메시 머티리얼 |
| `IMC_Default` | BP_MikaCharacter Details | Input Mapping Context |

---

## 입력 액션 이름 (IMC에서 찾는 이름 기준)

`FindActionInIMC()`가 이름에 포함된 문자열로 자동 탐색함.

| 액션 이름 | 동작 |
|---|---|
| `Move` | 이동 |
| `Look` | 시점 회전 |
| `Jump` | 점프 / 트래버설 |
| `Run` | 달리기 |
| `Aim` | 조준 |
| `Shoot` | 발사 / 미카 펀치 충전 |
| `Crouch` | 웅크리기 |
| `Reload` | 재장전 |
| `Weapon1` / `Weapon2` / `Weapon3` | 무기 교체 |
| `Throw` | 수류탄 |
