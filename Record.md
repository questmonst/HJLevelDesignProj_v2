# Record.md — 설계 결정 및 작업 이력

이 파일은 새 시스템 추가 시 설계 이유(ADR), 게임 핵심 루프, 주요 작업 이력을 기록한다.

---

## 핵심 루프 (Core Loop)

미카가 적을 향해 접근 → 펀치(수평) 또는 랜딩(수직)으로 근접 공격 → 미유가 사격각 확보 시 지원사격 → 아키라가 도주 → 추격 반복

플레이어 행동 축: **이동 → 충전 → 발동 → 쿨다운**
레벨 기작 축: **수직/수평 구조가 미카 스킬을 자연스럽게 강제**

---

## ADR 목록

---

### ADR-001: TraversalComponent — 애니메이션 트래버설 → 포물선 점프 전환

**날짜**: 2026-05-09

**결정**: 기존 MotionWarping 기반 트래버설 애니메이션 시스템을 제거하고, `LaunchCharacter` 포물선 점프로 대체.

**이유**:
- 트래버설 애니메이션이 없는 상태에서 MotionWarping 설정 비용이 불필요하게 높음
- 포물선 점프가 공중에서도 재발동 가능해 레벨 기작(수직 이동)과 더 잘 맞음
- 목표점 높이(`LandingHeightOffset`)를 블루프린트에서 직접 조정 가능

**구조**:
- `DetectTraversal()`: 전방 벽 스윕 → 상단 탐색 → 착지 좌표 계산
- `TryTraversal()`: 감지 성공 시 포물선 속도 계산 후 `LaunchCharacter`
- `bCanTraverse`: Tick에서 매 프레임 캐싱 → HUD에서 직접 읽기 가능

**주요 UPROPERTY**:
- `LandingHeightOffset` (default -30cm): 목표점 Z 낮추기
- `JumpArcHeight` (default 80cm): 포물선 최고점 추가 높이
- `JumpCooldown` (default 0.5s): 공중 연속 발동 방지

---

### ADR-002: Generic Team Agent — ACharacterBase에 구현

**날짜**: 2026-05-09

**결정**: `IGenericTeamAgentInterface`를 `ACharacterBase`에 구현하고 `TeamID` UPROPERTY로 팀 결정.

**이유**:
- `ACharacterBase`에 이미 `TeamID` UPROPERTY가 있었음 → 별도 레이어 불필요
- 모든 캐릭터 클래스가 `ACharacterBase` 상속 → 한 곳에서 관리
- `AEnemyAIController`는 `AAIController`가 이미 인터페이스를 상속하므로 `GetGenericTeamId()`만 오버라이드

**팀 ID 배분**:
- `TeamID = 0`: 미카(플레이어), 미유(아군 NPC) — 기본값이 0이라 별도 설정 불필요
- `TeamID = 1`: `AEnemyCharacter` 생성자에서 설정 → 모든 적 클래스에 자동 적용

**효과**: `AEnemyAIController`의 AIPerception `bDetectEnemies = true`가 실제로 플레이어를 탐지하기 시작함.

---

### ADR-003: IDestructibleObject — 미카 펀치 즉각 파괴 인터페이스

**날짜**: 2026-05-09

**결정**: `IDestructibleObject` 인터페이스를 `Characters/Base/IDestructible.h`에 정의하고, `ADestructibleCover`에 구현.

**이유**:
- 미카 펀치가 환경 오브젝트와 적을 다르게 처리해야 함
  - 환경 오브젝트: 데미지 누적 없이 **즉각 파괴**
  - 적 캐릭터: `ApplyDamage`로 체력 감소
- 인터페이스 방식을 쓰면 `DestructibleCover` 외에 추후 다른 오브젝트(포탑, 소품 등)도 동일하게 확장 가능

**호출 흐름**: `PunchHitbox Overlap` → `Cast<IDestructibleObject>` 성공 시 `Execute_DestroyByPunch()` → `OnCoverDestroyed()` → 콜리전 제거 + LifeSpan 설정

**BP 확장**: `DestroyByPunch`가 `BlueprintNativeEvent`이므로 BP에서 Chaos GC, VFX, 사운드 추가 가능.

---

### ADR-004: 적 체력바 — UEnemyHealthBarWidget으로 소유자 주입 + 델리게이트 구독

**날짜**: 2026-09-16

**결정**: 적 머리 위 체력바를 `UUserWidget` 서브클래스(`UEnemyHealthBarWidget`)로 만들고,
`ACharacterBase::BeginPlay`에서 `InitWidget()` → `BindToCharacter(this)`로 소유 캐릭터를 주입한다.

**이유**:
- `UWidgetComponent`가 만든 위젯은 `GetOwningPlayerPawn`이 **플레이어**를 반환한다 — WBP 안에서
  자기를 띄운 적 캐릭터에 접근할 경로가 없어서 ProgressBar Percent를 바인딩할 수 없었음
- 매 프레임 폴링 대신 `OnHealthChanged` 델리게이트를 위젯이 직접 구독 → Tick 비용 없음
- BP_DamageNumber(`InitDamage()` → `OnInitDamage` BP 훅)와 **같은 패턴** — 데이터는 C++이 밀어넣고
  비주얼만 BP가 처리

**표시 규칙** (CharacterBase::UpdateHealthBar):
- **Screen 스페이스** — 체력바를 "플레이어 UI"로 취급한다. 항상 지오메트리 위에 그려져
  벽·캐릭터에 파묻히지 않고, 위젯 크기가 월드 cm가 아니라 **픽셀**로 해석된다
  (World 스페이스에서는 SizeBox 200x18이 가로 2m 판때기가 되는 문제가 있었음)
- 스크린 스페이스는 거리와 무관하게 같은 크기라 먼 적까지 크게 떠서 지저분해진다.
  `Scale = Clamp(HealthBarRefDistance / 거리, HealthBarMinScale, 1.0)`로 거리에 따라 줄이되
  `HealthBarMinScale`(0.6)로 **가독성 하한**을 둔다. 1.0으로 두면 완전 고정
- 스케일은 컴포넌트가 아니라 위젯 `SetRenderScale`에 적용 — 컴포넌트 스케일을 매 프레임
  덮어쓰면 에디터에서 조정한 값이 무시되기 때문
- 마지막 피격 후 `HealthBarHideDelay`(3초) 경과 → `HealthBarFadeDuration`(0.5초) 동안
  `SetRenderOpacity` 1→0 **페이드 아웃**. 알파 0이면 `SetVisibility(false)`로 렌더에서 제외
- **페이드는 사라질 때만** 적용한다. 나타날 때 보간하면 대미지 숫자는 즉시 뜨는데 체력바만
  흐리게 올라와서 체력 감소가 늦은 것처럼 보였다 (실제 값은 델리게이트로 즉시 갱신됨)
- 뷰포트 밖/최대 거리 초과도 같은 알파 경로를 타므로 즉시 깜빡이지 않음

**지연바(PB_Delayed)**: 이전 체력 위치에서 `DelayedHoldTime`(0.35초) 버틴 뒤
`DelayedDrainSpeed`(초당 0.9 비율)로 현재 체력까지 따라 내려온다. `PB_Health`와의 차이 구간이
"방금 받은 대미지"로 읽힌다. 회복은 지연 없이 즉시 반영. WBP는 `GetDelayedPercent()`에 바인딩.
BP 타임라인 대신 `NativeTick`에 둔 이유는 "C++ 로직 / BP 비주얼" 원칙 때문.

**사망 연출**: `UPROPERTY(Transient, meta=(BindWidgetAnimOptional)) UWidgetAnimation* Anim_Death`로
WBP의 동명 애니메이션을 UMG가 자동 바인딩한다. `ACharacterBase::OnDeath`에서
`Bar->PlayDeathEffect()`를 직접 호출하므로 **BP 노드 배선이 전혀 필요 없다**.
(대안이던 "BP_AREnemy에서 GetUserWidgetObject → Cast → PlayAnimation"은 MCP로 DynamicCast
타깃 클래스를 지정할 수 없어 수동 작업이 필요했고, 캐스트 의존도 생긴다)
주의: 액터는 `DeathEffectDuration` 후 Destroy되므로 애니 길이보다 짧으면 연출이 잘린다.

**위젯 수명주기 함정 (중요)**: `UWidgetComponent`는 **숨겨질 때 슬레이트 위젯을 해제하고
다시 보일 때 재구성한다.** 그때 `NativeDestruct`가 불리므로, 여기서 소유자 포인터까지 지우면
바가 한 번 사라진 뒤의 피격이 영영 반영되지 않는다(첫 타만 정상 동작). 3중으로 막는다:
1. `NativeDestruct`는 구독만 해제하고 `OwnerCharacter`는 유지
2. `NativeConstruct`에서 기억해둔 소유자로 재구독
3. `NativeTick`에서 소유자의 실제 비율과 비교해 어긋나면 보정 — 위젯이 새 인스턴스로
   생성돼 구독을 놓쳐도 표시가 틀어진 채 남지 않는 **안전망**
추가로 `ACharacterBase::ShowHealthBar`가 피격마다 `BindToCharacter`를 다시 호출한다
(중복 구독은 함수 자체가 정리). 지연바는 재바인딩 시 리셋하지 않는다 — 연속 피격 중
매번 따라잡히면 대미지 구간이 안 보이기 때문.

**BP 확장점**: `OnHealthUpdated(New, Old)`(피격 플래시 등), `ACharacterBase::OnDeathEffect()`(추가 연출용)

---

### ADR-005: 적 사격 — C++ BT 태스크 + AI 조준을 컨트롤러 시점으로

**날짜**: 2026-09-17

**결정**: `UBTTask_FireAtTarget`(C++)이 `AEnemyCharacter::FireAtTarget/StopFiring`을 호출하고,
`AWeaponBase`의 AI 조준을 `GetActorEyesViewPoint()` 기준으로 바꾼다.

**이유**:
- 사격 로직은 `AEnemyCharacter::FireAtTarget()`에 이미 있었다. 빠진 건 BT에서 부를 경로뿐.
  BP 태스크로 만들면 캐릭터마다 Cast 노드 배선이 필요하고, MCP는 Cast 타깃 지정이 불가능하다.
  C++ 태스크는 BT 노드 목록에 바로 뜬다
- 기존 AI 조준은 `GetActorLocation/GetActorRotation` — 액터 로테이션은 **피치가 항상 0**이라
  위아래를 못 쏘고, 발사 지점이 캡슐 중심(허리)이었다. 수직 구조가 핵심인 이 게임에서 치명적.
  `HitscanFire` / `ProjectileFire` / `GrenadeFire` 3경로 모두 수정

**태스크 동작**: 사거리(`AttackRange`)·시야(`LineOfSightTo`) 확인 → 사격 + `OnAttack()` BP 훅 →
`FireDuration`(0이면 `AttackCooldown`) 유지 → `StopFiring`. 실패 시 상위 Sequence가 끊겨 접근 단계로 복귀.
`AbortTask`에서도 반드시 `StopFiring` — 총구가 안 멈추는 사고 방지.
조준은 태스크 책임이 아니다 — 앞선 `Rotate to face BB entry`/`Move To`가 폰을 타겟으로 돌린다.

---

### ADR-006: 발사 몽타주 — Additive 시퀀스 + ABP 최종단 UpperBody 슬롯

**날짜**: 2026-09-17

**결정**: 발사 시퀀스를 **Additive(Local Space, Base = 자기 자신 0프레임)** 로 두고,
`ABP_Riflegirl2_mika` AnimGraph의 최종 `BlendListByBool` → `Slot 'UpperBody'` → `Output Pose`로 얹는다.
앉은 상태는 `APlayerCharacter::SelectFireMontage()`가 `FireMontageCrouch`로 분기(비어 있으면 기본값).

**이유**:
- 몽타주가 안 보였던 원인은 재생 코드가 아니라 **AnimGraph에 Slot 노드가 없던 것**이었다.
  슬롯 매니저에 `UpperBody`가 등록돼 있는 것과 ABP가 그 슬롯을 읽는 것은 별개
- 일반(비 Additive) 애니를 슬롯에 넣으면 전신을 덮어써 걷기·앉기·조준 피치(ModifyBone)가 풀린다.
  Additive는 0프레임 대비 차이만 더하므로 **Layered blend per bone 없이도** 하체와 조준 자세가 유지된다
- `Apply Additive` 노드는 쓰지 않는다 — Slot 노드가 Additive 몽타주를 입력 포즈 위에 자동으로 얹는다.
  별도 분기로 만들면 비어 있는 입력이 레퍼런스 포즈가 되어 오히려 꼬인다
- 미유처럼 BP에서 `Play Anim Montage`를 분기하는 방식은 캐릭터마다 그래프 복사가 필요하고
  C++ 분기와 중복되므로 채택하지 않음 (재생 속도 노출 아이디어만 추후 DA로 흡수 가능)

**함정 — 루트 본 스케일 (중요)**: 미카 리타겟 애니는 루트 본 `survivor_teenangst_arm`의 스케일이
1.0으로 구워져 있으면 캐릭터가 **1/100로 축소**된다(7월 `fix_mika_root_scale.py`로 앉기 12종 픽스).
앉아쏴가 작아졌던 원인은 몽타주가 픽스 이후 만든 복제본 `Crouch_AimIdle_Shoot_mika1`을 참조했기 때문.
Additive는 기준 프레임과 상쇄돼 증상이 가려지므로, 서서쏴 `AimIdle_Shoot_mika`의 루트 트랙도 제거해
`ThuggedAnims` 시퀀스 24개 전부 루트 트랙 없음으로 통일. **새로 리타겟/복제한 애니는 반드시 확인할 것.**

**미유 참고**: 미유는 `APlayerCharacter`가 아니라 `SelectFireMontage()`를 못 쓴다. 우선순위 6에서 같은 구조로 이전.

### ADR-007: 적 재장전·엄폐 — C++ BT 노드 모음 + UEnemyDataAsset

**날짜**: 2026-09-17

**결정**: 적 AI 전용 BT 노드를 `Source/.../AI/`에 모으고(FireAtTarget도 이동), 적 행동 수치는
`UEnemyDataAsset`(클래스 1개, 적 종류마다 에셋 1개)로 뺀다. 재장전 판단은 서비스가 블랙보드 `bNeedsReload`에
올리고, BT는 기본 `Blackboard Based Condition`으로 분기한다. 엄폐 위치는 EQS로 찾는다.

**추가된 노드**
| 노드 | 역할 |
|---|---|
| `Fire At Target` (수정) | `FireDuration`(기본 5초) 동안 사격. 연사 무기는 무기 타이머, 단발·점사는 `IsFireReady()`마다 재발사. 탄창 비면 실패 + `bNeedsReload=true` |
| `Reload Weapon` | 재장전 시작 → 완료까지 대기 → `bNeedsReload=false` |
| `Set Move Mode` | Walk / Run / Crouch. 속도는 캐릭터(EnemyData) 값 |
| `Clear Blackboard Value` | 키 비우기 (엔진 기본 노드 없음) |
| `Find Patrol Location` | PatrolOrigin 주변 NavMesh 랜덤 지점 → PatrolLocation |
| `Update Weapon State` (서비스) | 잔탄 비율 → `bNeedsReload`. 루트 Selector에 부착 |
| `EnvQueryContext_BlackboardTarget` | EQS에서 "타겟(미카)" 컨텍스트. 엄폐 판정 트레이스의 기준 |

**이유**
- **DA 분리 기준**: 연사 속도·대미지·탄창은 이미 `UWeaponDataAsset`에 있다. 적 DA에는 "행동"(이동 속도, 교전 거리,
  재장전 기준)만 둔다. 통합 DA 하나는 적 6종에서 비대해지고, 무기별 DA는 무기 DA와 중복된다
- **서비스 + BB 키**: 데코레이터가 무기 액터를 직접 보게 하려면 C++ 데코레이터 + 재평가 요청 코드가 필요하다.
  BB에 올리면 Observer aborts가 엔진 기본 동작으로 처리된다. 서비스 틱 간격(0.2초) 공백은
  사격 태스크가 탄이 비는 순간 키를 직접 세워서 메운다
- **장전 중 키 유지**: 서비스는 장전 중에 `bNeedsReload`를 내리지 않는다. 내리면 재장전 분기가 abort되어
  엄폐 중에 뛰쳐나간다. 내리는 건 `Reload Weapon` 완료 시점 하나뿐
- **무한 예비탄**: 적이 탄약 부족으로 무력화되면 전투가 김빠진다. `bInfiniteReserveAmmo`로 장전 직전 예비탄 보충
- **FireAtTarget 기본값 0→5초**: 기존 0은 "AttackCooldown 동안"이라는 오해 소지 있는 동작이었다. 사격 유지 시간은
  BT 노드에서 직접 정한다. `AttackCooldown`·`AttackDamage`는 이제 어디서도 쓰지 않는다
  (TODO(무기 DA와 중복): 자식 적 생성자 정리 시 함께 제거)

**권장 BT 구조** (루트 Selector에 `Update Weapon State` 서비스)
```
Selector
 ├ [bNeedsReload Is Set, aborts both] Sequence_Reload
 │    Set Move Mode(Run) → Run EQS Query(EQS_FindCover → CoverLocation)
 │    → Move To(CoverLocation) → Set Move Mode(Crouch) → Reload Weapon
 │    → Set Move Mode(Walk) → Clear Blackboard Value(CoverLocation)
 ├ [bCanSeeTarget Is Set, aborts both] Sequence_Fire   (+ Set default focus 서비스 = TargetActor)
 │    Set Move Mode(Walk) → Move To(TargetActor, 반경 < AttackRange) → Fire At Target
 └ Sequence_Patrol
      Find Patrol Location → Move To(PatrolLocation) → Wait
```
Reload 분기를 Fire보다 **왼쪽(우선순위 높음)** 에 둬야 사격 중 탄이 떨어졌을 때 abort된다.

**사격 패턴 (추가)**: 3초 사격 / 2초 휴식은 코드 없이 `Fire At Target(3) → Wait(2)`. 확률 스트레이핑은
`BTDecorator_RandomChance`(엔진에 확률 데코레이터 없음) + `Simple Parallel`(메인 = Fire At Target, 보조 = EQS_Strafe → Move To, Allow Strafe).
RandomChance는 진입 시 한 번만 굴린다 — Observer aborts를 막아 실행 중인 분기가 재판정으로 끊기지 않게 함.

**행동 성향 (추가)**: 사격/휴식 시간(일반·제압 따로)과 확률(스트레이핑·제압·엄폐 이동)은 `UEnemyDataAsset`에 둔다.
`Fire At Target`·`Random Chance`의 `Use Enemy Data`가 켜져 있으면 적 종류별 값을 읽어, 같은 BT로 AR·MG·DMR 성향 차이를 낸다.
무기 DA가 아닌 적 DA에 두는 이유: 무기 DA는 미카도 공유하고, 적 1종 = 무기 1종이라 결과가 같다.
- `Fire At Target`은 "사격 → 휴식"을 한 번 수행한다(별도 Wait 불필요). Vector 키(`TargetLocation`)면 제압 사격 — 시야 확인 생략
- **잊기**: `AEnemyAIController::Tick`에서 경계 중 ① 스폰 지점에서 `LeashDistance` 초과 ② `ForgetTime` 동안 타겟 정보 없음이면
  `TargetActor`·`TargetLocation`·경계를 지운다 → 순찰 분기가 스폰 지점 주변으로 복귀시킨다.
  Leash로 잊은 직후 미카가 계속 보이는 상태면 Perception이 새 이벤트를 안 보내 재감지되지 않는다(시야를 잃었다 다시 잡으면 재감지) — 추격 한계로 의도된 동작
- **피격 즉시 감지** (갱신): `AEnemyCharacter::TakeDamage` → `UAISense_Damage::ReportDamageEvent`. 컨트롤러에 Damage 감각 추가.
  피격 시 `TargetActor`·경계를 세우고, 공격자가 시선(LOS) 안이면 `bCanSeeTarget`도 즉시 켜 교전에 들어간다(시야각 밖이어도).
  Damage 감각은 일회성 이벤트라 "감지 해제"가 오지 않으므로, `DamageEngageGraceTime`(1초) 안에 시야 감각이 이어받지 못하면
  컨트롤러가 `bCanSeeTarget`을 끈다. `bCanSeeTarget`은 시야 감각만 켜고 끈다 — 청각은 경계·위치만 갱신(기존엔 청각도 켜던 버그 수정)
- 시야를 잃을 때 `TargetLocation`을 `Stimulus.StimulusLocation`(마지막 목격 위치)으로 갱신 — 제압 사격 조준점

**함정**
- EQS 엄폐 트레이스는 Item Height Offset을 **앉은 눈높이(~60)**, Context Height Offset을 타겟 가슴 높이(~120)로.
  서 있는 높이로 재면 앉았을 때 머리 위로 넘어오는 낮은 엄폐물을 못 고른다
- EQS 실패(엄폐물 없음) 시 Sequence가 끊긴다. 제자리 재장전 폴백은 Reload 분기 안에 Selector를 한 겹 더 둔다

### ADR-008: 비전투 체력 회복 — UHealthRegenComponent + ACharacterBase::Heal

**날짜**: 2026-09-17

**결정**: 마지막 피격 후 `RegenDelay`초가 지나면 `RegenInterval`(N)초마다 `RegenAmount`(M)씩,
최대 체력 × `RegenCapRatio`까지만 회복한다. 상한 위에서 맞으면 회복하지 않는다. 수치는 `DA_Mika`의 `Stats|Regen`.
회복 중에는 `OnRegenStarted` / `OnRegenStopped` 델리게이트로 UI가 체력바를 깜빡인다.

**이유**
- **컴포넌트**: `PlayerCharacter.cpp`가 480줄로 한도에 가깝고, 미유·적에게도 재사용 가능해야 한다
- **`Heal()` 단일 경로**: 회복이 `CurrentHealth`를 직접 바꾸면 `OnHealthChanged` 방송이 빠질 수 있다.
  감소는 `TakeDamageCustom`, 증가는 `Heal` — 체력 변경 경로를 둘로 고정 (SSOT)
- **타이머 기반**: 틱 없이 `OnDamaged` 구독 → 지연 타이머 → 반복 타이머. 피격마다 둘 다 리셋
- **깜빡임은 UI 몫**: 컴포넌트는 상태만 알린다. 연출(UMG 애니)은 WBP가 결정 — C++이 위젯 구조를 모르게

**UI 연결**: `WBP_PlayerHealthBar`에서 `Get Owning Player Pawn → Cast To BP_Mika → Get Health Regen`
→ `Bind Event to OnRegenStarted`(Play Animation, Loop 0) / `OnRegenStopped`(Stop Animation).

### ADR-009: 미카 ABP 몽타주 레이어 — 슬롯 하나 + 캐시 포즈 + 조건 분기 (연습용 실용 구조)

**날짜**: 2026-09-18

**결정**: 발사·펀치·수류탄 몽타주를 모두 `DefaultGroup.UpperBody` **슬롯 하나**에 넣고, `ABP_Riflegirl2_mika` AnimGraph 끝단에서
"상체만 / 전신 / 척추 보정"을 **bool 조건**으로 고른다.

```
[이동 로코모션] → Save Cached 'LocoPose'
Use 'LocoPose' → Slot 'UpperBody' → Save Cached 'SlotPose'

Use 'LocoPose' → Base ┐
Use 'SlotPose' → Blend0 ┘ Layered blend per bone(Spine1, Mesh Space Rotation) ─→ False ┐
Use 'SlotPose' ────────────────────────────────────────────────────────────→ True  ├ Blend Poses by bool
                                         Active = Is Dashing OR bIsThrowingGrenade ┘
  → Local To Component → ModifyBone Spine1·Spine2(척추 조준, Alpha = Is Charging OR bIsPreparingThrow)
  → Component To Local → Output Pose
```

| 상태 | 결과 |
|---|---|
| 평소·충전·수류탄 준비 | 하체 = 로코모션(조준 걷기), 상체 = 몽타주 |
| 대시 펀치·수류탄 던지기 | **전신 = 몽타주 원본** |
| 충전·수류탄 준비 | + 척추가 카메라 상하(Aim Spine Pitch)를 따라감 |

EventGraph의 ABP 변수 `Is Aiming` = `bIsAiming OR bIsChargingPunch OR bIsDashing OR bIsPreparingThrow` —
조준 걷기 하체 선택. **AnimGraph의 Is Aiming 사용처를 각각 고치지 말고 이 한 곳만** 수정한다.

**이유 (레벨 디자인 연습용이라 이 수준에서 멈춤)**
- 슬롯이 하나라 같은 슬롯 그룹 규칙으로 "펀치가 시작되면 발사 몽타주가 끊김"이 자동으로 된다
- 상태 추가 비용이 "bool 하나 OR"로 끝난다 — 새 동작(랜딩 등)도 같은 자리에 조건만 추가
- 몽타주 슬롯은 **반드시 `DefaultGroup.UpperBody`**. 이름과 달리 전신 분기도 이 슬롯 결과(SlotPose)를 쓴다.
  `DefaultSlot`으로 만들면 어느 분기에서도 안 보인다(2026-09-18 던지기 몽타주에서 실제로 발생)

**함정**
- ABP 변수 `Is Aiming`(AnimGraph용)과 C++ `APlayerCharacter::bIsAiming`(사격↔펀치 분기·조준 카메라)은 **별개**.
  C++ 쪽을 펀치용으로 켜면 발사 버튼이 사격으로 간다
- 척추 보정은 반드시 레이어·전신 분기 **뒤**. 앞에 두면 몽타주(Mesh Space)가 덮어 무시된다
- 캐시 포즈를 거치지 않고 슬롯 Source를 비워 두면 레퍼런스 포즈(T자)가 섞인다

**근본 해결(나중에 필요하면)**: 동작별 슬롯 분리(UpperBody/FullBody) + **Linked Anim Layer**로 전투 레이어를 별도 ABP로 분리,
상태는 bool 여러 개 대신 enum 하나(`EMikaActionState`)로. 동작 종류가 더 늘거나 다른 캐릭터(미유)와 레이어를 공유할 때 전환.

---

## 작업 이력

| 날짜 | 작업 | 비고 |
|---|---|---|
| 2026-05-09 | TraversalComponent 포물선 점프 전환 | ADR-001 |
| 2026-05-09 | bCanTraverse Tick 캐싱 추가 | HUD 연동용 |
| 2026-05-09 | Generic Team Agent 구현 | ADR-002 |
| 2026-05-09 | IDestructibleObject 인터페이스 추가 | ADR-003 |
| 2026-05-09 | 모든 적 클래스 C++ 구조 완성 | BP + BT 작업만 남음 |
| 2026-05-09 | CLAUDE.md 코드 구조 실제 코드 기준으로 정정 | ACharacterBase, AAkiraEnemy 위치 수정 |
| 2026-09-16 | 적 체력바 위젯 클래스 + 페이드/최소 크기 보정 | ADR-004 |
| 2026-09-16 | 체력바 Screen 스페이스 전환 + 지연바(chip damage) 추가 | ADR-004 갱신 |
| 2026-09-16 | 페이드인 즉시화 + Anim_Death 자동 바인딩 재생 | ADR-004 갱신 |
| 2026-09-16 | 체력바 재구성 시 구독 유실 버그 픽스(3중 방어) | ADR-004 갱신 |
| 2026-09-17 | 적 사격 BT 태스크 + AI 조준 눈높이·피치 | ADR-005 |
| 2026-09-17 | 발사 몽타주 Additive + UpperBody 슬롯, 앉아쏴, 루트 트랙 정리 | ADR-006 |
| 2026-09-17 | 적 재장전·엄폐 BT 노드 + UEnemyDataAsset, 사격 5초 유지·단발 재발사 | ADR-007 |
| 2026-09-17 | 확률 데코레이터(스트레이핑 사격용) + 미카 비전투 체력 회복 컴포넌트 | ADR-007 보강, ADR-008 |
| 2026-09-17 | 적 행동 성향 DA(사격 패턴·확률), 제압 사격, 타겟 잊기, 피격 방향 반응 | ADR-007 보강 |
| 2026-09-17 | 적 피격 즉시 감지(AISense_Damage) + 시야 인계 확인, 청각이 bCanSeeTarget 켜던 버그 수정 | ADR-007 갱신 |
| 2026-09-18 | 미카 ABP 몽타주 레이어 구조 기록(상체/전신/척추 보정 분기), 수류탄 투척 노티파이·조준·총 숨김 | ADR-009 |
