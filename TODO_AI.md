# TODO_AI (AI 에이전트가 이어서 할 일 · 사람에게 알릴 데이터)

> AI(Claude)가 **나중에 C++을 수정해야 하는 것**과 **사람이 에디터에서 데이터를 입력·확인해야 하는 것**을 모은다.
> 사람의 일반 수동 작업은 [Todo_Human.md](Todo_Human.md), 설계 근거는 [Record.md](Record.md).
> 완료 시 `[x]` 체크. 새 항목은 날짜와 함께 추가.

---

## A. 사람이 입력·확인할 데이터

### 적 AI (2026-09-17)
- [x] `WeaponData_AR` 등 무기 DA → **Audio › ReloadSound** 지정 후 재장전 소리 확인
- [x] `EnemyData_AR` → **Forget › ForgetTime** / **LeashDistance** 조정
- [x] `BP_EnemyAIController` 생성 → **DamageEngageGraceTime** 확인
- [x] 피격 즉시 감지 PIE 확인
- [x] 미카 사망 PIE 확인

### EQS 엄폐 판정 — 제자리 재장전 재발 (레벨 디자인 진행하며 확인)
증상: 엄폐물로 이동하지 않고 제자리에서 재장전 후 앉았다 일어남 (`Sequence_AlreadyCover`가 항상 성공).
확인된 것: Visibility 채널은 캐릭터 캡슐·메시가 Ignore, 무기 메시는 NoCollision → **자기 몸에 막히는 문제 아님**. Bullet 채널로 바꾸면 캐릭터가 Block이라 오히려 악화.
- [ ] **(유력)** `EQS_IsInCover` Trace의 **Item Height Offset 60 → 0 ~ -20**. `Current Location`은 바닥이 아니라 캡슐 중심(~90cm) 기준이라 +60이면 1.5m 높이에서 트레이스됨. (`EQS_FindCover`의 Grid는 NavMesh 투영이라 60 유지)
- [ ] 타겟(`TargetActor`)이 빈 상태에서 재장전하면 Trace 테스트가 필터링을 건너뛰어 **항상 통과**(엔진 동작) — 타겟을 잊은 뒤 재장전 시 재현되는지 확인
- [ ] 두 EQS의 Trace 테스트가 **Test Purpose = Filter Only**, **Bool Match 체크**인지 확인 (Python으로 읽기 불가해 AI가 검증 못 함)

---

## B. AI가 나중에 C++로 수정할 것

- [ ] **적이 죽은/사라진 타겟을 계속 쏨** — `AEnemyAIController::UpdateForget`에 "TargetActor 무효 또는 `IsDead()`면 즉시 ForgetTarget" 조건 추가. 플레이어가 사망 시 Destroy되지 않게 바뀌어 Perception이 계속 보고 있음
- [ ] **BP 컨트롤러에서 시야·청각 수치가 안 먹힘** — `SightRadius`·`LoseSightRadius`·`PeripheralVisionAngleDeg`·`HearingRange`가 생성자에서만 SenseConfig에 복사되어 BP 기본값이 반영되지 않음. `OnPossess`(또는 BeginPlay)에서 값 재적용 + `AIPerception->RequestStimuliListenerUpdate()`. 수치를 EnemyData로 옮길지도 함께 결정 (2026-09-17, BP_EnemyAIController 생성 시 발견)
- [ ] `AEnemyAIController::PeripheralVisionAngleDeg` 툴팁 수정 — "양쪽 합산"이 아니라 엔진상 **한쪽 반각** (60 = 좌우 합 120°)
- [ ] `AEnemyCharacter::AttackCooldown` / `AttackDamage` 제거 — 어디서도 안 씀(무기 DA와 중복). 자식 적 생성자(`AREnemy.cpp` 등) 대입도 함께 정리
- [ ] `BTTask_FindPatrolLocation`의 `"PatrolLocation"` 하드코딩 → `AEnemyCharacter::BBKey_PatrolLocation` 상수로 (다른 BB 키와 통일)
- [ ] 체크포인트 재시작 — `APlayerCharacter::OnDeath_Implementation`의 `TODO(체크포인트 재시작 미구현)`. `PlayerCharacter.cpp`가 492줄이라 GameMode 또는 별도 컴포넌트에 구현
- [ ] `WeaponBase.cpp` 719줄 — 500줄 규칙 초과(이전부터). 발사/재장전/그립 정렬 등으로 파일 분리 검토

### 보류 (필요해지면)
- [ ] 적 조준(Ironsights) 모션 — `bIsAiming` + `BTService_SetAiming` + ABP `Blend Poses by bool`·`Aim_Space_Ironsights`. 2026-09-17 "적은 디테일 불필요"로 보류
- [ ] EQS 대신 C++ 엄폐 태스크(`Find Cover Location` / `Is In Cover`) — EQS 판정 문제가 계속되면 전환 검토
