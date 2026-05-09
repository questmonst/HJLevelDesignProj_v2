# 개발 진행 현황 (2026-05-09)

## 완료된 것

### 클래스 구조
- `ACharacterBase` — 체력/팀ID/사망 공통 기반
- `APlayerCharacter` — 미카의 부모 (이동, 조준, 무기, 수류탄, 트래버설, 중력)
- `AMikaCharacter` — 미카 펀치(충전→대시→히트박스) + 미카 랜딩(다이브) 구현
- `ANPCCharacter` — 적/아군 NPC 공통 기반
- `AEnemyCharacter` — BT 기반 적 AI 기반 클래스
- 적 타입 6종 골격: `AAREnemy`, `AShotgunEnemy`, `ASniperEnemy`, `AMGEnemy`, `AShieldEnemy`, `ALargeSweeperEnemy`
- `AAkiraEnemy` — 스크립트 이동 + 페이즈 변환 이벤트
- `AMiyuCharacter` — 지원사격 (타겟 지정/LoS 체크/쿨다운)
- `ABaseAirUnit` → `ADroneEnemy`, `AHelicopterEnemy` 골격
- `ABaseTurret` → `ASmallTurret` 골격

### 플레이어 기능
- 기본 TPS 이동: 걷기 / 달리기 / 앉기 / 점프
- 조준(ADS) + 허리 Pitch 회전 (Spine Transform Modify Bone용)
- Turn-in-place (좌/우 임계값 기반 회전)
- 엄폐 피크(Cover Peek) 카메라 오프셋
- 크로스헤어 퍼짐 (이동/발사/조준 연동)
- 낙하 상태 감지: `bIsFalling`, `CurrentFallSpeed`, 하드랜딩 판정
- 착지 이벤트: `OnLanding(bHardLanding)` — ABP에서 오버라이드해 몽타주 재생
- 미카 펀치: MinChargeTime ~ ForcedMaxChargeTime 충전 → 대시 속도 선형 보간 → 히트박스 오버랩 처리
- 미카 랜딩: `MikaLanding()` + 지면 거리 체크 `CanTriggerLanding()`
- 중력 방향 변경: `SetGravityDirection()` / `ResetGravity()`
- 트래버설: Hurdle / Vault / Mantle / Climb (MotionWarping 연동)

### 무기 시스템
- `AWeaponBase` — 히트스캔/투사체, 자동/반자동, 탄창, 재장전, 머즐VFX, 발사음, 히트VFX
- `AProjectileBase` — 물리 투사체
- `AGrenadeBase` — 퓨즈 타이머, 폭발 반경
- 수류탄 궤도 스플라인 시각화
- 무기 다중 슬롯 + 교체 딜레이
- `UWeaponDataAsset`, `UProjectileDataAsset`, `UMikaDataAsset`, `UHUDDataAsset` — 블루프린트 노출 수치

### 환경 요소
- `ADestructibleCover` — 체력 기반 파괴 (OnCoverDestroyed BP 이벤트)
- `AFallResetTrigger` / `AFallResetTarget` — 낙사 리셋 (페이드 아웃→텔레포트→페이드 인)
- `ASplineTrackActor` — 스플라인 따라 메시 배치 (롤러코스터 레일 등)

### 기타
- `ECharacterAction` 비트마스크 — AllowedActions 시스템으로 행동 제한
- `AEnemyAIController` — BT 실행, 블랙보드 키 공유
- `AHJGameMode` 기반 설정

---

## 미완성 / 다음에 할 것

### 우선순위 높음
| 항목 | 메모 |
|------|------|
| 미카 펀치 환경 파괴 연동 | `IDestructible` 인터페이스 또는 `ADestructibleCover` 직접 호출 |
| 미카 랜딩 자동 발동 (롤러코스터 루프) | 루프 구간 진입 시 `MikaLanding()` 자동 트리거 |
| 아키라 BT 구간별 세팅 | 레벨 01(도주+거점사격), 02(열차 간 이동), 03(PTP 점프), 04(3페이즈) |
| 레벨 04 페이즈 관리자 | 스포트라이트 발판, 아키라 3페이즈 전환 조건 |

### 우선순위 중간
| 항목 | 메모 |
|------|------|
| 구간 C — 회전하는 차량 | 드럼통 회전 액터 + 플레이어 낙하 감지 |
| 구간 D — 중력 전환 연출 | 시퀀서 카메라 90도 회전 → `SetGravityDirection()` 호출 타이밍 |
| 구간 E — 수직 낙하 공간 전환 | 문 통과 트리거 → 낙하 강제 구간 |
| 미유 BP 연동 | `OnSupportFire` 구현 (몽타주 + 발사체) |
| 적 타입별 BT 태스크 | AR/샷건/저격/MG/쉴드/스위퍼 각자 전술 패턴 |

### 우선순위 낮음
| 항목 | 메모 |
|------|------|
| 바람에 따른 머리카락 흩날리기 | Physics Asset or Groom + Wind Actor |
| 스포트라이트 이동 발판 시스템 | 레벨 04 전용, 조명과 콜리전 동기화 |
| GAS 연동 (레벨 04 아키라) | 어빌리티 시스템으로 페이즈 전환 관리 |

---

## 코드 구조 (현재)

```
Source/GameAnimationSample2/
├── Characters/
│   ├── Base/
│   │   ├── CharacterBase.h/.cpp         ← 체력/사망
│   │   └── CharacterTypes.h             ← ECharacterAction 비트마스크
│   ├── Player/
│   │   ├── PlayerCharacter.h/.cpp       ← TPS 기반, 무기/수류탄/트래버설/중력
│   │   ├── MikaCharacter.h/.cpp         ← 펀치/랜딩
│   │   ├── MikaDataAsset.h/.cpp
│   │   └── TraversalComponent.h/.cpp
│   └── NPC/
│       ├── NPCCharacter.h/.cpp
│       ├── EnemyCharacter.h/.cpp        ← BT 기반 AI
│       ├── EnemyAIController.h/.cpp
│       ├── MiyuCharacter.h/.cpp         ← 지원사격
│       ├── AkiraEnemy.h/.cpp            ← 스크립트 이동 + 페이즈
│       ├── AREnemy / ShotgunEnemy / SniperEnemy
│       ├── MGEnemy / ShieldEnemy / LargeSweeperEnemy
│       ├── BaseAirUnit / DroneEnemy / HelicopterEnemy
│       └── BaseTurret / SmallTurret
├── Weapons/
│   ├── WeaponBase.h/.cpp
│   ├── WeaponDataAsset.h/.cpp
│   ├── ProjectileBase.h/.cpp
│   ├── ProjectileDataAsset.h/.cpp
│   └── GrenadeBase.h/.cpp
├── Environment/
│   ├── DestructibleCover.h/.cpp
│   ├── FallResetTrigger.h/.cpp
│   ├── FallResetTarget.h/.cpp
│   └── SplineTrackActor.h/.cpp
├── UI/
│   └── HUDDataAsset.h/.cpp
└── Core/
    └── HJGameMode.h/.cpp
```
