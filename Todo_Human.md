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

**⑦ 적 사격** (ADR-005)
- [x] `BT_AREnemy`의 발견 Sequence에서 `Wait` 자리를 **`Fire At Target`**(Target Key = `TargetActor`)으로 교체
- [x] Move To의 Acceptable Radius < `AttackRange`(1200) — 넘으면 사거리 밖이라 태스크가 계속 실패
- [x] 적이 사거리 안에서 사격
- [ ] 벽 뒤에서는 안 쏨 / 높은 곳의 미카를 올려다보며 쏨
- [ ] (정리) `ABP_AREnemy` Jump 그래프의 `Cast To Ue4ASP_Character → SET Jump Button Down` 죽은 노드 삭제

### AR 적 AI 패턴 — 단계별 (ADR-007, 2026-09-17)

> 한 층씩 쌓고 PIE로 확인한 뒤 다음 단계로. 디버그: PIE에서 `'` → 넘버패드 1(BT)·2(EQS)·3(Perception).
> 사람이 넣을 데이터·AI가 고칠 C++은 [TODO_AI.md](TODO_AI.md).

- [x] **0단계 준비** — EnemyData_AR 연결, Use Controller Rotation Yaw, BB TargetActor Base Class, NavMesh, 재장전 몽타주 슬롯(UpperBody)
- [x] **1단계 기본 사격 + 순찰** — 사격 1초 / 휴식 4초(EnemyData), Find Patrol Location → Move To(Radius 50) → Wait
- [x] **2단계 재장전** — `[bNeedsReload] Reload Weapon`, 재장전 사운드
- [x] **3단계 EQS 에셋** — `EQS_IsInCover`, `EQS_FindCover` (V2AI)
- [~] **4단계 엄폐 재장전** — `Selector_reload` 구성 완료. 제자리 재장전 재발 문제는 TODO_AI A 참고(레벨 디자인하며 확인)

**5단계 — 경계 (피격 방향, 마지막 위치 추적, 제압 사격, 잊기)** — 전투와 순찰 **사이**에 추가
```
[bIsAlerted Is Set, aborts lower] Sequence_Alert      ← 서비스: Set default focus(TargetLocation)
  Set Move Mode (Walk)
  Selector [Force Success]
    └ [Random Chance: Suppress] Fire At Target (Target Key = TargetLocation, Pattern = Suppress)
  Move To (TargetLocation, Radius 100)                 ← 마지막 목격 위치로 추적
  Wait (3 ± 1)
```
- [ ] 적이 "빨리 잊는" 문제의 주원인이 이 분기 부재 — 만들면 시야를 잃어도 마지막 위치로 추적함
- [ ] 확인: 뒤에서 쏘면 돌아봄 / 숨으면 마지막 위치에 제압 사격(확률) / 마지막 위치까지 추적 / `ForgetTime` 후 순찰 복귀 / `LeashDistance` 넘게 끌고 가면 포기

**6단계 — 전투 변주 (스트레이핑, 개활지 엄폐 이동)**
- [ ] `EQS_Strafe` 생성 — Generator **Points: Donut**(Inner 200, Outer 450, Center Querier) / Pathfinding(Path Exist, Filter) / Trace(Context `EnvQueryContext_BlackboardTarget`, **Bool Match 해제** = 보이는 곳) / Dot(Querier→Item vs Querier→BlackboardTarget, Absolute, Filter Max 0.4) / Distance(To BlackboardTarget, Filter 600~1200). Run Mode = Single Random Item from Best 25%
- [ ] 1단계의 `Fire At Target` 자리를 아래로 교체:
```
Selector_Combat
 ├ Sequence (엄폐 중): Run EQS Query(EQS_IsInCover)
 │    Selector
 │     ├ [Random Chance: Strafe] Simple Parallel(메인 Fire At Target / 보조 Run EQS Query(EQS_Strafe→StrafeLocation) → Move To(Allow Strafe))
 │     └ Fire At Target (Attack)
 ├ [Random Chance: Seek Cover] Sequence (개활지):
 │    Run EQS Query(EQS_FindCover→CoverLocation)
 │    Simple Parallel(메인 Move To(CoverLocation, Allow Strafe) / 보조 [Loop] Fire At Target)
 └ Fire At Target (Attack)
```
- [ ] BB_Enemy에 `StrafeLocation`(Vector) 키 추가
- [ ] 확인: 개활지에서 엄폐물로 이동하며 사격 / 엄폐 중 가끔 옆으로 움직이며 사격 / 이동 애니가 미끄러지지 않는지(블렌드스페이스 방향 지원)

**7단계 — 적 종류별 성향**
- [ ] `EnemyData_AR` 복제 → `EnemyData_MG`, `EnemyData_DMR` 등. 사격 패턴(Burst/Rest)·확률(Strafe/Suppress/SeekCover)만 다르게 (예: MG = 사격 5초/휴식 1초/스트레이핑 0%)
- [ ] BT는 공유 — 노드의 `Use Enemy Data`가 켜져 있으면 적마다 다르게 행동

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
