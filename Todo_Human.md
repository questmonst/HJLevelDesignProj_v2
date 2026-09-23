# Todo_Human (근시일 수동 작업)

> 사람이 에디터/플레이로 직접 해야 하는 단기 작업 목록.
> 전체 수동 작업 레퍼런스는 [progress_human.md](progress_human.md) 참고.
> 완료 시 `[x]` 체크.

---

## 1. `[x]` 유탄 발사기 테스트 — 완료

GL 발사 체인 동작 확인 완료. (탄도/폭발/VFX 정상)

> 값 조정은 `GrenadeData_Launcher`(폭발/VFX/크기) 와 `WeaponData_GL`(발사 속도/탄약)에서.

---

## 2. `[~]` 수류탄 손에 부착 후 던지기 — 거의 완료

코드 구현됨: 조준 시작 시 손 소켓에 부착(생성 FX → 본체 VFX) → 떼면 발사. 준비 완료 전엔 던지기 보류.

남은 것(사람):
- [ ] **생성 FX ↔ 본체 부착 간격 직접 조정** — `GrenadeData_Throwable.SpawnToProjectileDelay`(기본 0.5초)를 생성 FX 길이에 맞춰 튜닝
- [ ] 던지기 애니메이션(몽타주) 제작 후 연결 — 들기/던지기 모션 (지금은 무기와 같은 손 소켓이라 겹쳐 보임)

> 관련 코드: `PlayerCharacter_Grenade.cpp`, `GrenadeBase.cpp`.

---

## 3. `[x]` 앉기(Crouch) — 완료

**C++/입력:**
- `bCanCrouch=true`, `bCrouchMaintainsBaseLocation=true` (PlayerCharacter.cpp 생성자)
- 앉기 **토글**로 변경 — `Crouch` 입력(Started) → `ToggleCrouch()`가 `bIsCrouched` 보고 앉기<->서기 전환 (한 번 누르면 앉고 다시 누르면 일어섬)

**애니메이션:**
- 앉기 애니 12종 `RTG_RifleGirl2Mika`로 리타겟 → 스케일 축소버그는 `fix_mika_root_scale.py`로 픽스
- `BS_Mika_AimWalkCrouch`, `BS_Mika_Walk2RunCrouch` 앉기 블렌드스페이스 제작
- `ABP_Riflegirl2_mika`: `isCrouching` bool 기반 앉기 포즈 분기 추가 (`Get Is Crouched → SET isCrouching`)

> PIE 확인 완료 — 토글 앉기 + 앉기 애니 정상 동작.

---

## 4. `[x]` 캐릭터 DA에 반동(발사) 모션 할당 — 완료

발사 몽타주를 무기별 → **캐릭터 공통**으로 옮김 (`MikaDataAsset.FireMontage`).

- [x] `MikaData`의 **FireMontage** = `AM_Shoot_mika`, **FireMontageCrouch** = `AM_Shoot_Crouch_mika`
- [x] 서서/앉아/걸으며/위아래 조준 사격 모두 확인
- 구조: 시퀀스를 **Additive(Local Space)** 로 두고 ABP 최종 포즈 직전 `Slot 'UpperBody'`에서 얹음 (ADR-006)
- [ ] `Crouch_AimIdle_Shoot_mika1` — 참조 0개인 복제본. 확인 후 삭제

> C++ 완료. 무기 DA의 FireMontage는 제거됨 — 이제 캐릭터 DA에서만 설정.

---

## 5. `[~]` 적 캐릭터 추가 — AR 적 1종 진행중

기본 적 + 아키라(보스) 등 적 캐릭터 제작.

**AR 적 (완료된 부분):**
- [x] `BP_AREnemy` 생성 (`/Game/V2_HJContents/V2Character/Enemy/`, 부모 `AAREnemy`)
- [x] 마네킹 메시(`SK_Mannequin_Animstarter`) + `ABP_AREnemy`(AnimStarterPack 복제) + `BP_Weapon_AR` 자동 장착
- [x] 히트/사망 = C++ 완비 (`TakeDamage→Die→OnDeath`), TeamID=1

**AR 적 — 완료:**
- [x] `BP_AREnemy` 테스트 레벨에 배치 → 미카가 쏴서 히트/사망 확인
- [x] 머리 위 체력바 + 대미지 숫자 (ADR-004)

### AR 적 AI — 진행할 순서

> C++은 전부 완성돼 있다. Perception(시야 2000 / 청각 1200 / 시야각 60°), 블랙보드 키 기록,
> 팀 ID까지 `AEnemyAIController`에 구현됨. **에셋만 만들면 된다.**

**① 테스트 레벨에 NavMesh 배치**
- [x] `NavMeshBoundsVolume`을 적이 움직일 범위에 씌우고 `P` 키로 초록 영역 확인
- 이게 없으면 BT의 모든 이동 태스크가 **조용히 실패**한다. 반드시 먼저

**② `BB_Enemy` 생성** (`/Game/V2_HJContents/V2AI/`) — **[x] MCP로 생성 완료**
- [x] 키 5개 추가 (이름은 `EnemyCharacter.cpp`의 `BBKey_*` 상수와 일치 확인됨)
- [ ] **`TargetActor`의 Base Class를 `Actor`로 지정** — 이 필드만 Python에 노출이 안 돼
      수동. 블랙보드 에디터에서 `TargetActor` 선택 → Key Type ▸ Base Class 드롭다운

  | 키 | 타입 |
  |---|---|
  | `TargetActor` | Object (Base Class = Actor) |
  | `TargetLocation` | Vector |
  | `bCanSeeTarget` | Bool |
  | `bIsAlerted` | Bool |
  | `PatrolOrigin` | Vector |

**③ `BT_AREnemy` 생성**
- [x] 블랙보드에 `BB_Enemy` 지정
- [x] 최소 구조 (이동까지 동작 확인):

  ```
  Selector
   ├ [Blackboard: bCanSeeTarget == true] Sequence
   │    → Move To (TargetActor, Acceptable Radius = 사거리)
   │    → Rotate to face BB entry (TargetActor)
   │    → Wait (사격 간격)
   └ Sequence
        → Move To (PatrolOrigin)
        → Wait
  ```

**④ `BP_AREnemy` 연결**
- [x] `AI Controller Class` = `AEnemyAIController`
- [x] `Auto Possess AI` = `Placed in World or Spawned`
- [x] BehaviorTree 슬롯에 `BT_AREnemy`

**⑤ 적 앉기 ABP 배선**
- [x] `ABP_AREnemy` EventGraph의 `Cast To BP_EnemyBase`(구세대, 우리 적에선 실패)를
      `Cast To Character → Is Crouched`로 교체. locomotion은 이미 동작함
      (MCP는 DynamicCast 타깃 클래스 지정 불가라 수동)

**⑥ PIE 테스트**
- [x] 적이 미카를 발견하면 접근하는지
- [ ] 안 되면 `P`(Show Navigation), `'`(AI Debug)로 블랙보드 값이 실제로 채워지는지 확인

**⑦ 적 사격 — C++ 완료, PIE 확인 필요** (ADR-005)
- [ ] `BT_AREnemy`의 발견 Sequence에서 `Wait` 자리를 **`Fire At Target`**(Target Key = `TargetActor`)으로 교체
- [ ] Move To의 Acceptable Radius < `AttackRange`(1200) — 넘으면 사거리 밖이라 태스크가 계속 실패
- [ ] 적이 사거리 안에서 사격 / 벽 뒤에서는 안 쏨 / 높은 곳의 미카를 올려다보며 쏨
- [ ] (정리) `ABP_AREnemy` Jump 그래프의 `Cast To Ue4ASP_Character → SET Jump Button Down` 죽은 노드 삭제

**나머지 적:**
- [ ] Shotgun/Sniper/MG/Shield/LargeSweeper 등 자식 클래스 + 에셋 연결
      (AR 적 BT 구조가 검증되면 복제해서 파라미터만 바꾸는 식으로)

> 상세 목록·BT·BB 키는 [progress_human.md](progress_human.md) §1·§7·§8 참고.

---

## 7. `[x]` 적 체력바 / 대미지 숫자 — 완료

`WBP_EnemyHealthBar`(부모 `UEnemyHealthBarWidget`) 제작 완료. 검정 빈칸 → 빨강 지연바 →
초록 현재체력 3겹 구성, `Anim_Death` 사망 연출까지 동작 확인.

> 수치 조정은 `BP_AREnemy` 디테일의 `Character|UI` 카테고리에서.
> 설계·함정은 [Record.md](Record.md) ADR-004 참고.

---

## 6. `[ ]` 미카 앉기 카메라 DA 조정

앉기 카메라 보정 C++ 완료 (스프링암 `TargetOffset.Z`, 전투 카메라와 독립).

- [ ] `DA_Mika`(UMikaDataAsset)에서 **Camera > Crouch** 값 조정:
  - `CrouchCameraZOffset` — 앉기 시 카메라 올리는 높이 (기본 40)
  - `bSmoothCrouchCamera` — 보간(true)/즉시(false) 선택
  - `CrouchCameraInterpSpeed` — 보간 속도
- [ ] `Movement > CrouchWalkSpeed` — 앉아 이동 속도 (기본 200) 조정

> C++ 완료(빌드 반영됨). DA 값만 만지면 됨.

---
3단계: EQS 에셋 만들기
EQS_IsInCover
Generator: Current Location
Test: Trace → BlackboardTarget, Bool Match = true
EQS_FindCover
Generator: Points: Grid
Tests: Pathfinding / Trace(true) / Distance to Target(Min 500) / Distance to Querier(Inverse Linear)
✅ 확인: 레벨에 EQS Testing Pawn을 놓고 Query Template를 지정해. 엄폐물 뒤 지점이 초록으로 뜨는지 봐.

Testing Pawn은 블랙보드가 없어서 BlackboardTarget 컨텍스트가 비어. 이 단계에서만 Context를 Querier가 아닌 다른 액터로 임시로 바꿔서 확인하고, 확인이 끝나면 되돌려줘.
4단계: 엄폐 재장전
2단계에서 넣은 Reload Weapon 한 줄을 이걸로 교체:


 ├ [bNeedsReload Is Set, aborts both] Selector_Reload
 │   ├ Sequence: Run EQS Query(EQS_IsInCover) → Set Move Mode(Crouch) → Reload Weapon
 │   └ Sequence: Set Move Mode(Run) → Run EQS Query(EQS_FindCover→CoverLocation)
 │               → Move To(CoverLocation) → Set Move Mode(Crouch) → Reload Weapon → Set Move Mode(Walk)
✅ 확인:

개활지에서 탄이 떨어지면 엄폐물까지 뛰어가서 앉아 재장전하는지
이미 엄폐 중이면 그 자리에서 앉아 재장전하는지
5단계: 경계 (피격 방향, 제압 사격, 잊기)
Combat과 Patrol 사이에 추가:


 ├ [bIsAlerted Is Set, aborts lower] Sequence_Alert  ← 서비스: Set default focus(TargetLocation)
 │    Rotate to face BB entry(TargetLocation)
 │    [Random Chance: Suppress] Fire At Target(TargetLocation, Suppress)
✅ 확인:

뒤에서 쏘면 그쪽으로 도는지
미카가 숨으면 마지막 위치에 제압 사격하는지 (확률이라 여러 번 테스트)
ForgetTime(10초)이 지나면 순찰로 복귀하는지
스폰 지점에서 LeashDistance(2500)보다 멀리 끌고 가면 포기하는지
6단계: 전투 변주 (스트레이핑, 개활지 엄폐 이동)
EQS_Strafe 생성
Generator: Donut
Tests: Pathfinding / Trace(false) / Dot(Absolute, Max 0.4) / Distance
1단계의 Fire At Target 자리를 이걸로 교체:

Selector_Combat
 ├ Sequence(엄폐 중): Run EQS Query(EQS_IsInCover)
 │    Selector
 │     ├ [Random Chance: Strafe] Simple Parallel(메인 Fire At Target / 보조 EQS_Strafe → Move To(Allow Strafe))
 │     └ Fire At Target(Attack)
 ├ [Random Chance: Seek Cover] Sequence(개활지):
 │    Run EQS Query(EQS_FindCover→CoverLocation)
 │    Simple Parallel(메인 Move To(CoverLocation, Allow Strafe) / 보조 [Loop] Fire At Target)
 └ Fire At Target(Attack)
✅ 확인:

개활지에서 엄폐물로 이동하면서 쏘는지
엄폐 중에 가끔 옆으로 움직이며 쏘는지
이동 애니가 미끄러지지 않는지 (ABP_AREnemy 블렌드스페이스가 방향을 지원하는지)
7단계: 적 종류별 성향
EnemyData_AR을 복제해서 EnemyData_MG, EnemyData_DMR 등을 만들어.
사격 패턴과 확률만 다르게 줘. 예: MG는 사격 5초 / 휴식 1초 / 스트레이핑 0%.
BT는 하나를 공유해. Use Enemy Data가 켜져 있으면 적마다 다르게 행동해.
막히는 단계가 있으면 BT 스크린샷이랑 PIE 증상을 보내줘. MCP가 다시 연결되면 내가 BT 구성을 직접 읽고 PIE에서 확인할게.
---

## 바로 다음에 할 일 (미카 모션, 2026-09-18)

> 애니 리타겟(`RTG_Manny2Mika_v2`) → `fix_mika_root_scale.py` → 몽타주(슬롯 `DefaultGroup.UpperBody`) 만든 뒤 AI에 연결 요청. 몽타주 칸은 C++이 필요 (TODO_AI 참고)

- [ ] **`Jmp_BackAir`** → 미카 펀치 **반동(튕겨 나올 때)** 모션
- [x] **`Jump_Up_B` / `Jump_Down_B`** → **점프** 모션 (ABP AirLoco 완료)
- [ ] **`Land_Spawn_Wait`** → **30° 아래 펀치로 바닥 찍을 때(착지 공격)** 모션
- [ ] **`Land_Base_Wait`** → **고지대 일반 착지** 모션

## 나중에 할 일 (생각날 때)

> 급하지 않은 것. C++이 필요한 건 TODO_AI.md에도 함께 있음.

- [ ] **수류탄 던질 수 있는지 UI** — 위젯에서 `OnGrenadeThrowReadyChanged(bReady)` 바인딩. C++ 완료
- [ ] **펀치 충전 UI** — `OnPunchFullCharge` 바인딩 + 충전 게이지 `GetPunchChargeRatio()`(0~1). C++ 완료
- [ ] **펀치 풀 충전 대시 전용 VFX·SFX** — AI에 요청
- [ ] **카와이 리밋 조정** — `KawaiiPhysics/DA_KawaiiLimits_MikaSkirt`(골반·허벅지 구) / `DA_KawaiiLimits_MikaHair`(머리·목·상체 구). 반경·위치는 임시값이라 뚫리거나 뜨면 조정. 노드에는 이미 다 연결돼 있어서 에셋 하나만 고치면 전부 반영됨
- [ ] **카와이 피직스 세부 조정** — 미카 ABP AnimGraph 끝부분 카와이 노드 16개(치마 8·머리 8). 미유와 같은 기본값(Damping 1 / WorldDampingLocation 2 / WorldDampingRotation 0.8 / Stiffness 1 / Radius 1)으로 넣어둠. 흔들림·충돌 반경은 취향대로
- [ ] **미카 `LeftHand` 소켓 위치 조정** — 장전 중 총이 붙는 자리. 지금은 WeaponSocketLeft와 같은 값으로 임시 배치. 무기에 `LeftHandGrip` 소켓이 없으면 총 루트 기준으로 붙으니 소켓도 확인
- [x] **BGM 곡 넣기** — 동작 확인 완료 (전투 감지는 적 인지 상태로 자동) — `/Game/V2_HJContents/Data/BGMData`에 Explore·Combat·Boss 곡 지정 → 레벨 BP에서 BGM 서브시스템 `SetBGMData` → `StartBGM`, 전투 지점에서 `NotifyCombat`
- [ ] **펀치 반동 구르기 몽타주** — 만들어서 MikaData › Animation › `PunchReboundMontage`에 지정
- [ ] **적 AI Perception 청각 확인** — 총성만 소음을 낸다(`FireNoiseRange` 3000). 적 컨트롤러에 Hearing 감각이 켜져 있어야 반응
- [ ] **점프 확인** — `JumpAnimPlayRate`(MikaData › Fall, 현재 0.8) 조정, 착지 1번·즉시 나오는지. 끊겨 보이면 ABP Land 상태 Start Position 0.05 → 0.03/0. `LandAnticipationTime`은 0 유지(올리면 착지 루프)
- [ ] **점프 중 펀치 시 캐릭터가 화면 밖으로 나감** — 카메라는 캡슐 고정(랙 없음)이라 메시가 캡슐에서 벗어나는 것으로 추정. 추천 A: PIE 캡처로 캡슐/메시 위치 확인 후 메시 쪽 수정 (보완: `DashSpringArmLength`·`DashFOV` 키우기). AI에 요청
- [ ] (나중에) **수류탄 NS 수정** — `NS_Bomb_Projectile` 복제본에서 이미터 Loop Behavior = Infinite. 끝나면 AI에 `OnProjectileVFXFinished` 제거 요청
- [ ] (나중에) **보스 BP에 `bIsBoss` 체크** — 전투 BGM 판정에서 제외됨
- [ ] (보류) **치마 다리 관통** — 충돌 구를 다리 전체에 깔아도 결국 뚫림. 치마 8가닥이 각각 별도 카와이 노드라 면이 아니라 선으로 동작하는 한계. 리그에서 치마 본을 공통 부모 아래로 묶으면 본 제약(Bone Constraints)으로 면처럼 묶을 수 있음. 스윙 각도 제한은 각 노드 `PhysicsSettings.LimitAngle`(현재 치마 55도) — 더 크게 움직이길 원하면 이 값을 올리거나 0(무제한)
