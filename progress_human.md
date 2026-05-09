# 수동 작업 목록 (에디터에서 직접 해야 하는 것들)

> C++ 코드는 로직만 담당, 아래 항목들은 언리얼 에디터에서 수동으로 세팅해야 함.
> 완료된 항목은 `[x]`로 체크.

---

## 1. 블루프린트 자식 클래스 생성

각 C++ 클래스에서 블루프린트 자식을 만들어야 메시·애님·수치를 에디터에서 설정할 수 있음.

| BP 이름 (예시) | 부모 C++ 클래스 | 우선순위 |
|---|---|---|
| `[ ]` BP_Mika | AMikaCharacter | 최상 |
| `[ ]` BP_Miyu | AMiyuCharacter | 상 |
| `[ ]` BP_Akira | AAkiraEnemy | 상 |
| `[ ]` BP_AREnemy | AAREnemy | 중 |
| `[ ]` BP_ShotgunEnemy | AShotgunEnemy | 중 |
| `[ ]` BP_SniperEnemy | ASniperEnemy | 중 |
| `[ ]` BP_MGEnemy | AMGEnemy | 중 |
| `[ ]` BP_ShieldEnemy | AShieldEnemy | 중 |
| `[ ]` BP_LargeSweeperEnemy | ALargeSweeperEnemy | 중 |
| `[ ]` BP_DroneEnemy | ADroneEnemy | 하 |
| `[ ]` BP_HelicopterEnemy | AHelicopterEnemy | 하 |
| `[ ]` BP_SmallTurret | ASmallTurret | 하 |
| `[ ]` BP_WeaponAR | AWeaponBase | 상 |
| `[ ]` BP_WeaponShotgun | AWeaponBase | 중 |
| `[ ]` BP_WeaponSniper | AWeaponBase | 중 |
| `[ ]` BP_Grenade | AGrenadeBase | 중 |
| `[ ]` BP_DestructibleCover | ADestructibleCover | 상 |
| `[ ]` BP_FallResetTrigger | AFallResetTrigger | 상 |
| `[ ]` BP_FallResetTarget | AFallResetTarget | 상 |
| `[ ]` BP_SplineTrackActor | ASplineTrackActor | 중 |

---

## 2. 인풋 매핑 컨텍스트 (IMC)

`Content/Input/` 에 아래 에셋 생성 후 BP_Mika의 `IMC_Default` 슬롯에 연결.

PlayerCharacter.cpp 에서 이름 기준으로 자동 탐색하므로 **이름 철자 정확히** 맞춰야 함.

| `[ ]` IA 이름 | 타입 | 키 |
|---|---|---|
| `IA_Move` | Axis2D (WASD) | W/A/S/D |
| `IA_Look` | Axis2D (마우스) | Mouse XY |
| `IA_Jump` | Digital | Space |
| `IA_Sprint` | Digital (Hold) | LShift |
| `IA_Crouch` | Digital (Hold) | LCtrl |
| `IA_Aim` | Digital (Hold) | RMB |
| `IA_Fire` | Digital | LMB |
| `IA_Reload` | Digital | R |
| `IA_Weapon1` | Digital | 1 |
| `IA_Weapon2` | Digital | 2 |
| `IA_Weapon3` | Digital | 3 |
| `IA_GrenadeThrow` | Digital (Hold) | G |
| `IA_Punch` | Digital (Hold) | F |

---

## 3. 데이터 에셋 생성 및 수치 입력

| `[ ]` 에셋 이름 | 클래스 | 연결 위치 |
|---|---|---|
| `DA_Mika` | UMikaDataAsset | BP_Mika → MikaData 슬롯 |
| `DA_WeaponAR` | UWeaponDataAsset | BP_WeaponAR → WeaponData 슬롯 |
| `DA_WeaponShotgun` | UWeaponDataAsset | BP_WeaponShotgun → WeaponData 슬롯 |
| `DA_WeaponSniper` | UWeaponDataAsset | BP_WeaponSniper → WeaponData 슬롯 |
| `DA_ProjectileDefault` | UProjectileDataAsset | 각 BP_Weapon → ProjectileClass 슬롯 |
| `DA_HUD` | UHUDDataAsset | BP_Mika → HUDData 슬롯 |

---

## 4. 애니메이션 블루프린트 (ABP)

| 항목 | 작업 내용 |
|---|---|
| `[ ]` ABP_Mika 생성 | AMikaCharacter 스켈레톤 기준으로 새 ABP 생성 |
| `[ ]` 이동 상태머신 | Idle / Walk / Sprint / Crouch / Jump / Fall / Land |
| `[ ]` 조준 AimOffset 연결 | `AimYaw`, `AimSpinePitch` 변수를 ABP에서 읽어 Spine Modify Bone 적용 |
| `[ ]` Turn-in-place 연결 | `bIsTurningRight` / `bIsTurningLeft` 읽어서 TurnR/TurnL 애님 재생 |
| `[ ]` 낙하/착지 연결 | `bIsFalling`, `CurrentFallSpeed` → Fall 블렌딩. `bIsHardLanding` → 하드랜딩 몽타주 |
| `[ ]` 미카 펀치 슬롯 | UpperBody 슬롯 몽타주로 재생 (하체는 이동 계속) |
| `[ ]` 미카 랜딩 슬롯 | FullBody 슬롯 몽타주로 재생 |
| `[ ]` 트래버설 슬롯 | MotionWarping 노드 연결. `TraversalLanding`, `TraversalLedge` 워프 타겟 이름 맞출 것 |
| `[ ]` ABP_Enemy (공용 or 타입별) | 적 기본 이동/사격/사망 상태머신 |

---

## 5. 애니메이션 몽타주

| `[ ]` 몽타주 이름 | 설명 |
|---|---|
| `AM_MikaPunchCharge` | 충전 자세 루프 |
| `AM_MikaPunchDash` | 대시 공격 모션 |
| `AM_MikaLanding` | 낙하 → 착지 충격 |
| `AM_HardLand` | 하드랜딩 쓰러짐 → 일어남 |
| `AM_Hurdle` | 낮은 장애물 넘기 |
| `AM_Vault` | 중간 장애물 뛰어넘기 |
| `AM_Mantle` | 높은 장애물 매달려 올라가기 |
| `AM_Climb` | 2.5m급 장애물 클라임 |
| `AM_MiyuSupportFire` | 미유 지원사격 모션 |
| `AM_EnemyFire` | 적 기본 사격 |
| `AM_EnemyDeath` | 적 사망 |

### AnimNotify 세팅 (몽타주 안에서 추가)
| `[ ]` 위치 | 알림 이름 | 호출 함수 |
|---|---|---|
| 펀치 대시 끝 | AN_PunchEnd | `Allow(ECharacterAction::Punch)` 또는 쿨다운 리셋 |
| 트래버설 착지 순간 | AN_TraversalFinish | `TraversalComponent->FinishTraversal()` |
| 하드랜딩 일어남 끝 | AN_ResetHardLanding | `PlayerCharacter->ResetHardLanding()` |

---

## 6. 소켓 설정 (스켈레톤 에디터)

| `[ ]` 소켓 이름 | 위치 | 용도 |
|---|---|---|
| `hand_r` | 오른손 | 미카 PunchHitbox 부착 (C++에서 이미 참조) |
| `WeaponSocket` | 오른손 (무기 쥐는 위치) | `PlayerCharacter::WeaponAttachSocket` 에 입력할 이름 |
| `WeaponHolster` | 등 또는 허리 | 수납 시 소켓 (`WeaponHolsterSocket`) |
| `MuzzleSocket` | 총구 끝 | `WeaponBase::MuzzleSocketName` |

---

## 7. 비헤이비어 트리 (BT) — 에디터에서 제작

| `[ ]` BT 이름 | 연결 캐릭터 | 주요 태스크 |
|---|---|---|
| `BT_AREnemy` | BP_AREnemy | 순찰 → 감지 → 엄폐이동 → 사격 |
| `BT_ShotgunEnemy` | BP_ShotgunEnemy | 원거리 사격 → 일정 시간 후 돌격 |
| `BT_SniperEnemy` | BP_SniperEnemy | 고지대 고정 → 조준 → 사격 |
| `BT_MGEnemy` | BP_MGEnemy | 거점 고정 → 연속 사격 |
| `BT_ShieldEnemy` | BP_ShieldEnemy | 전진 → 쉴드 유지 → 근접 공격 |
| `BT_LargeSweeperEnemy` | BP_LargeSweeperEnemy | 근접 범위 넓게 휩쓸기 |
| `BT_Akira_Level01` | BP_Akira | 거점 이동 → 원거리 사격 → 플레이어 접근 시 도주 |
| `BT_Akira_Level02` | BP_Akira | 열차 간 이동 → 같은 열차면 스킬 |
| `BT_Akira_Level03` | BP_Akira | PTP 점프 (Sequencer 연동) |
| `BT_Akira_Level04` | BP_Akira | 3페이즈 자율 행동 |

> 각 BT는 해당 BP 캐릭터의 `BehaviorTree` 슬롯에 연결, `AIController`는 `BP_EnemyAIController` 지정

---

## 8. 블랙보드 에셋

`BB_Enemy` 하나 만들어서 모든 적 BT에서 공유.

| `[ ]` 키 이름 | 타입 | 용도 |
|---|---|---|
| `TargetActor` | Object | 플레이어 레퍼런스 |
| `TargetLocation` | Vector | 마지막으로 본 위치 |
| `bCanSeeTarget` | Bool | 현재 시야 여부 |
| `bIsAlerted` | Bool | 경계 상태 |
| `PatrolOrigin` | Vector | 순찰 기준 위치 |

> 키 이름은 `EnemyCharacter.cpp`의 `BBKey_*` 상수와 **정확히** 일치해야 함

---

## 9. 레벨 배치 (레벨 에디터)

### 레벨 01 — 지하철
| `[ ]` 작업 | 메모 |
|---|---|
| 열차 3개 나란히 배치 | 좌/중/우 루트 |
| 기울어진 열차 배치 | 발판 불안정 구간 B |
| 회전하는 차량 배치 | 드럼통 회전 액터 — 별도 BP 제작 필요 |
| 중력 전환 트리거 볼륨 배치 | 구간 D 진입 시 시퀀서 발동 |
| BP_FallResetTrigger/Target 쌍 배치 | 낙사 구역마다 |
| BP_DestructibleCover 배치 | 미카 펀치 파괴 가능 엄폐물 |
| 적 배치 (BP_AREnemy 등) | BT 슬롯 연결 |

### 레벨 02 — 롤러코스터
| `[ ]` 작업 | 메모 |
|---|---|
| BP_SplineTrackActor로 레일 제작 | 루프 포함 |
| 열차 메시를 스플라인 따라 이동시키는 BP 제작 | 타임라인 or Sequencer |
| 루프 구간 트리거 배치 | 미카 랜딩 자동 발동용 |
| 미유 고정 위치 배치 | BP_Miyu, 사격각 확보 가능한 지점 |
| 아키라 리셋 위치 배치 (BP_FallResetTarget) | 미유 사격 후 낙하 지점 |

### 레벨 03 — 수직 낙하
| `[ ]` 작업 | 메모 |
|---|---|
| 수직으로 기울어진 열차 배치 | 의자 = 발판 |
| 발판 겸 의자 메시 배치 | 하강 루트 설계 |

### 레벨 04 — 무대
| `[ ]` 작업 | 메모 |
|---|---|
| 무대 구조물 배치 | 추락하는 발판 |
| 스포트라이트 액터 배치 | 발판 콜리전과 동기화 필요 — BP 제작 필요 |
| 미유 객석 위치 배치 | |
| 아키라 페이즈별 이동 위치 마커 배치 | |

---

## 10. 시퀀서 (Sequencer)

| `[ ]` 시퀀스 이름 | 내용 |
|---|---|
| `SEQ_GravityTransition` | 구간 D: 카메라 90도 서서히 회전 → 지정 타이밍에 레벨 BP에서 `SetGravityDirection()` 호출 |
| `SEQ_Akira_Level03` | 아키라 PTP 점프 경로 |
| `SEQ_FinaleLight` | 레벨 04 클라이맥스: 어둠 → 전체 조명 ON |

---

## 11. 레벨 블루프린트 연결

| `[ ]` 작업 | 메모 |
|---|---|
| 중력 전환 트리거 → 시퀀서 재생 → `SetGravityDirection()` 호출 체인 | 구간 D |
| 미유에게 `SetTarget(AkiraRef)` 호출 시점 지정 | 레벨 02 진입 시 |
| 아키라 페이즈 전환 조건 체크 및 `OnPhaseChange(Phase)` 호출 | 레벨 04 |
| 루프 구간 진입 → `MikaLanding()` 자동 호출 | 레벨 02 |

---

## 12. 게임모드 / 월드 세팅

| `[ ]` 항목 | 설정값 |
|---|---|
| World Settings → GameMode | BP_HJGameMode |
| BP_HJGameMode → Default Pawn Class | BP_Mika |
| BP_HJGameMode → HUD Class | WBP_HUD (위젯 제작 후 연결) |
| BP_HJGameMode → Player Controller Class | 기본 또는 커스텀 |
| AIController Class (각 BP_Enemy Detail) | BP_EnemyAIController |

---

## 13. HUD 위젯 (UMG)

| `[ ]` 위젯 이름 | 내용 |
|---|---|
| `WBP_HUD` | 크로스헤어, 체력바, 탄약 표시, 수류탄 카운트 |
| `WBP_Crosshair` | `HUDDataAsset` 값 읽어서 크기/퍼짐/색상 적용 |

> `CurrentCrosshairSpread` (PlayerCharacter 변수)를 바인딩해서 실시간 퍼짐 반영

---

## 14. VFX / 사운드 에셋 연결

| `[ ]` 항목 | 연결 위치 |
|---|---|
| 히트 Niagara 시스템 | BP_Weapon → `HitVFX` 슬롯 |
| 머즐 Niagara 시스템 | BP_Weapon → `MuzzleVFX` 슬롯 |
| 발사 사운드 | BP_Weapon → `FireSound` 슬롯 |
| 미카 랜딩 충격 VFX/사운드 | `OnExplode` 또는 ABP 노티파이에서 스폰 |
| 폭발 VFX/사운드 | BP_Grenade → `OnExplode` BP 이벤트에서 처리 |
| 엄폐물 파괴 VFX | BP_DestructibleCover → `OnCoverDestroyed` BP 이벤트 |

---

## 15. Physics Asset (스켈레톤 에디터)

| `[ ]` 항목 | 메모 |
|---|---|
| 미카 래그돌 Physics Asset 세팅 | 사망 시 래그돌 전환용 |
| 머리카락 Physics Asset (바람 흩날리기) | Groom 또는 Physics Asset 바람 영향 설정 — 우선순위 낮음 |

---

## 16. Motion Warping 타겟 이름 확인

TraversalComponent.h 에서 코드가 참조하는 워프 타겟 이름:

| `[ ]` 워프 타겟 이름 | 용도 |
|---|---|
| `TraversalLanding` | 착지 위치 (Hurdle/Vault/Climb 공통) |
| `TraversalLedge` | 가장자리 손 위치 (Mantle/Climb) |

> ABP의 MotionWarping 노드에서 이 이름으로 타겟을 참조하도록 설정할 것

---

## 요약 — 지금 당장 해야 할 것 (최소 플레이 가능 상태까지)

```
1. [ ] BP_Mika 생성 → 미카 메시, ABP, IMC_Default, MikaData 연결
2. [ ] IMC_Default + IA 생성
3. [ ] DA_Mika, DA_HUD 생성
4. [ ] ABP_Mika 기본 상태머신 (이동/낙하/착지)
5. [ ] BP_WeaponAR 생성 → 미카 DefaultWeaponClasses[0]에 연결
6. [ ] 테스트 레벨에 BP_Mika 배치 + WorldSettings GameMode 연결
7. [ ] 적 하나(BP_AREnemy) + BT_AREnemy + BB_Enemy 만들어서 전투 테스트
```
