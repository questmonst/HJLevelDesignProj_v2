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

### 미카 펀치 애니메이션 (2026-09-17)
- [x] 빌드 (총 숨기기 + 펀치 몽타주 재생)
- [x] ABP 상체 전용 레이어 (Layered blend per bone + UpperBody 슬롯)
- [x] CLazyAnimpack(UE5 마네킹) → 미카 리타겟 (`RTG_Manny2Mika_v2`) — `ThuggedAnims/MikaPunch/`에 `Attack_Hand_Ready/1R/2L/PwR_Mika` 4종, 루트 스케일 트랙 제거 완료
- [ ] **수직 펀치(랜딩)** — `JAttack_AirFront`(`Attack_Jump_Air/`)를 수정해 사용하기로 함(2026-09-17). `RTG_Manny2Mika_v2`로 리타겟 → `fix_mika_root_scale.py` → 구간 잘라 `LandingDiveMontage`(낙하, Auto Blend Out 해제)·`LandingImpactMontage`(착지). 필요 시 `JAttack_AirDrop_Keep/End`도 비교
- [x] **상체만 적용** (CLazy 펀치는 달리기 하체가 섞여 있음): `ABP_Riflegirl2_mika` AnimGraph — 최종 `부울로 포즈 블렌딩` → Save Cached Pose `LocoPose` → `Layered blend per bone`(Base = LocoPose, Blend 0 = LocoPose → 기존 `Slot 'UpperBody'`, Bone `ValveBiped_Bip01_Spine1`, Mesh Space Rotation Blend 체크) → 출력 포즈. 현재 UpperBody 슬롯은 최종 포즈 전체 위라 일반 애니는 다리까지 덮음
- [ ] 몽타주 생성(슬롯 `DefaultGroup.UpperBody`) → `MikaData` › **Animation › Punch**
  - `PunchChargeMontage` = `Attack_Hand_Ready_Mika` — Blend In 0.05, **Enable Auto Blend Out 해제**(마지막 준비 자세 유지)
  - `PunchDashMontage` = `Attack_Hand_PwR_Mika` — 세그먼트 Anim Start/End Time으로 앞 딜레이·뒤 멈춤 제거, 주먹이 대시 0.1~0.15초에 뻗도록 Play Rate 조정
  - 랜딩 2종은 애니 확보 후
- [x] **빌드 완료** — 대시 길이 = 펀치 몽타주 실제 재생 길이(Rate Scale 반영). `MikaData` › Punch › `bDashDurationFromMontage`(기본 true), 끄면 기존 `DashDuration`. 대시가 길어지면 히트박스·대시 카메라(FOV/스프링암)도 그만큼 유지되니 몽타주 Anim End Time으로 길이 조절 (2026-09-17)
- [x] **빌드 완료** — 총 복구 시점을 대시 종료(0.25초) → **펀치·착지 몽타주 길이만큼 대기 후**로 변경 (2026-09-17, "총이 안 사라진다" 대응)
- [ ] 충전·펀치 중 하체 = **조준 걷기 블렌드스페이스** (앉기 A안 대신 채택, 2026-09-17): ABP EventGraph의 **ABP 변수** `Is Aiming = 캐릭터 bIsAiming OR bIsChargingPunch OR bIsDashing`. A안(`isCrouching` OR 충전)을 적용했다면 되돌릴 것. **C++ 캐릭터의 bIsAiming은 건드리지 말 것**(펀치↔사격 분기·조준 카메라). 상체가 카메라 쪽으로 비틀리면 척추 조준 ModifyBone 2개 Alpha를 충전 중 0으로
- [ ] 충전 중 고개 들기: ABP AnimGraph 출력 직전에 `Transform (Modify) Bone` — Bone `ValveBiped_Bip01_Head1`(부족하면 `Neck1`에도 절반), Rotation Mode **Add to Existing**, Space **Bone Space**, Alpha Bool = `bIsChargingPunch`(Blend In/Out 0.15). 회전 축은 기존 척추 조준 노드처럼 **Roll**이 상하(피치) — -15~-25도부터 부호 바꿔가며 조정
- [ ] PIE: 충전 시작 시 총 숨김 → 펀치 몽타주 끝날 때 / 착지 몽타주 끝날 때 / 짧게 눌러 미발동 시 총 복구
- [ ] ABP 전신/상체 분기 — 대시 중(또는 제자리 충전)엔 전신, 이동하며 충전할 땐 상체만: `Slot 'UpperBody'` 출력을 Save Cached Pose `SlotPose` → `Layered blend per bone`(Blend 0 = SlotPose) → `Blend Poses by bool`(True = SlotPose, False = Layered 결과, Blend Time 0.1) → 출력 포즈. bool = `bIsDashing OR (bIsChargingPunch AND NOT ShouldMove)` (BP_Mika 변수를 ABP EventGraph에서 읽기, C++ 불필요)

### 미카 펀치 · 사격 정지 (2026-09-18)
- [x] 빌드 (적 사망 시 사격 중단·무기 드롭, 미카 사격 릴리즈 처리)
- [ ] PIE: 적 사망 시 무기가 발밑에 떨어져 주울 수 있는지 / 발사가 즉시 멈추는지 / 조준 사격 중 조준만 풀고 버튼을 떼도 총구가 멈추는지 / 사격 중 펀치 충전 진입 시 멈추는지
- [ ] `MikaData` › Punch | Camera — 대시 중 확대: `DashFOV` 105 → **70**(조준과 동일), `DashSpringArmLength` 420 → **180~250**. 대시가 짧아 보간이 안 끝나면 `CameraInterpSpeed` 10 → 15~20 (충전과 공용)
- [x] ABP: **펀치(대시) 중 전신 원본 모션** — `Slot 'UpperBody'` 출력을 Save Cached Pose `SlotPose` → `Layered blend per bone`(Blend 0 = SlotPose) → `Blend Poses by bool`(True = SlotPose 전신, False = Layered 상체만, Blend Time 0.1, bool = `bIsDashing`) → 출력 포즈
- [x] ABP: **충전 중 허리를 조준 방향으로** — Layered blend **뒤**에 `Transform (Modify) Bone`(Bone `ValveBiped_Bip01_Spine2`, Add to Existing, Bone Space, Alpha Bool = `bIsChargingPunch`), Roll ← `Aim Spine Pitch * -0.6`(기존 척추 조준 노드와 같은 축·부호). 몽타주가 Spine1 위를 Mesh Space로 덮으므로 레이어 앞에 두면 무시됨
- [ ] ABP: 대시 중 상체 젖히기 — 위와 같은 노드, Alpha Bool = `bIsDashing`, Roll -10~-20 (전신 분기를 쓰면 원본 애니에 이미 포함되어 불필요할 수 있음)

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
- [ ] **낙하 리셋을 AI(아키라)에도 적용** — `AFallResetTrigger::OnTriggerOverlap`이 PlayerController 없는 캐릭터를 무시함. 컨트롤러 없어도 텔레포트(페이드는 플레이어만). 아키라도 미카와 같은 스포트라이트(리셋 타겟)로 리셋 — 분리 불필요. 30m_DailyTask.md 3-4 (2026-09-17)
- [ ] **미카 펀치 넉백** — 현재 `OnPunchHitboxOverlap`은 `ApplyDamage`만. 1-1에서 아키라 낙하 원인으로 확정(스크립트 낙하 + 넉백). 히트 시 `LaunchCharacter` + 수치 DA 노출. 30m_DailyTask.md 3-5 (2026-09-17)

### 보류 (필요해지면)
- [ ] 적 조준(Ironsights) 모션 — `bIsAiming` + `BTService_SetAiming` + ABP `Blend Poses by bool`·`Aim_Space_Ironsights`. 2026-09-17 "적은 디테일 불필요"로 보류
- [ ] 미카 약/강 펀치 분기 — `MikaData`에 `PunchLightMontage`(`Attack_Hand_1R_Mika`)·`PunchPowerChargeRatio` 추가, 충전 비율 기준 미만이면 약펀치 몽타주. 2026-09-17 "충전·강펀치만 먼저"로 보류
- [ ] EQS 대신 C++ 엄폐 태스크(`Find Cover Location` / `Is In Cover`) — EQS 판정 문제가 계속되면 전환 검토
