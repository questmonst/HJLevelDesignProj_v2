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
