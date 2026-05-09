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

## 작업 이력

| 날짜 | 작업 | 비고 |
|---|---|---|
| 2026-05-09 | TraversalComponent 포물선 점프 전환 | ADR-001 |
| 2026-05-09 | bCanTraverse Tick 캐싱 추가 | HUD 연동용 |
| 2026-05-09 | Generic Team Agent 구현 | ADR-002 |
| 2026-05-09 | IDestructibleObject 인터페이스 추가 | ADR-003 |
| 2026-05-09 | 모든 적 클래스 C++ 구조 완성 | BP + BT 작업만 남음 |
| 2026-05-09 | CLAUDE.md 코드 구조 실제 코드 기준으로 정정 | ACharacterBase, AAkiraEnemy 위치 수정 |
