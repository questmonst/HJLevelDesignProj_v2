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
- [x] **빌드 완료** — 잔탄 없음·장전 중·단발 쿨다운일 때 발사 버튼을 눌러도 반동 몽타주 재생 안 함 (`APlayerCharacter::StartFire`, 2026-09-18). PIE: 빈 탄창으로 발사 시 빈 총 소리만 나는지
- [x] **빌드 완료** — 수류탄: 생성 연출(`SpawnToProjectileDelay`) 완료 전에 떼면 **취소**(수류탄 소모 없음, 들고 있던 것 제거). 완료 후 떼야 발사. 기존 "준비 전 요청 보류 → 준비되면 자동 발사" 로직 제거 (2026-09-18). PIE: 짧게 눌렀다 떼면 개수 유지·아무것도 안 날아감 / 충분히 누른 뒤 떼면 발사
- [ ] ABP: 대시 중 상체 젖히기 — 위와 같은 노드, Alpha Bool = `bIsDashing`, Roll -10~-20 (전신 분기를 쓰면 원본 애니에 이미 포함되어 불필요할 수 있음)

### 수류탄 애니메이션 (2026-09-18) — C++ 빌드 완료
- [ ] `WpAttack_Hand_1R`(CLazy `Attack_Wp_Swing_Run/`) 리타겟(`RTG_Manny2Mika_v2`) → `fix_mika_root_scale.py` → 몽타주 2개로 자르기(슬롯 `DefaultGroup.UpperBody`)
  - 준비: 팔을 뒤로 젖힌 구간까지, **Enable Auto Blend Out 해제**(누르고 있는 동안 유지)
  - 던지기: 팔을 앞으로 뿌리는 구간부터 끝까지
- [ ] `MikaData` › **Animation › Grenade**: `GrenadePrepareMontage`, `GrenadeThrowMontage`
- [ ] ABP(선택): `bIsPreparingThrow`·`bIsThrowingGrenade`로 하체 조준 걷기(`Is Aiming` OR 조건에 추가)·척추 조준 보정(Alpha OR 조건에 추가)
- [ ] (관찰 후 결정) 수류탄은 버튼을 떼는 순간 날아감 — 던지기 애니의 손 뻗는 프레임과 어긋나면 발사 지연(`GrenadeReleaseDelay`) 또는 Anim Notify 발사를 C++로 추가

### 수류탄 클래스 DA 이전 · SFX (2026-09-18)
- [x] 빌드
- [ ] `MikaData` › Grenade › **`GrenadeClass`** = `BP_Grenade_Thrown` 지정 (비워두면 BP_Mika에 있던 값 유지)
- [ ] `GrenadeData_Throwable` › **SFX**: `SpawnSound`(생성, 손 따라다님) / `BounceSound`(튕김·구름) / `BounceSoundMinSpeed`(기본 150, 이하 충돌은 무음) / `ExplosionSound`
- [ ] (선택) `GrenadeData_Launcher`에도 `BounceSound`·`ExplosionSound`. `SpawnSound`는 무기 발사음과 겹치니 비워두는 것 권장
- [ ] PIE: 생성 소리가 손을 따라가는지 / 튕길 때마다 소리, 거의 멈춰 굴러갈 땐 조용한지 / 폭발 소리 / 준비 전 취소 시 생성 소리가 이상하게 남지 않는지

### 수류탄 투척 노티파이 · 폭발 타이밍 · 생성 FX 크기 (2026-09-18) — C++ 빌드 완료
- [x] `WpAttack_Hand_1R_mika`(`ThuggedAnims/MikaGrenade/`) 루트 스케일 트랙 제거
- [ ] 던지기 몽타주에서 손이 수류탄을 놓는 프레임에 **Add Notify › Grenade Release** 추가 (이 순간 발사 + `ThrowSound`). 노티파이가 없으면 몽타주 끝에 자동 발사
- [ ] `GrenadeData_Throwable` › SFX › **`ThrowSound`**
- [ ] `GrenadeData_Throwable` › VFX › **`ExplosionBodyLingerTime`**(기본 0.1초) — 폭발 VFX가 뜨기 전에 수류탄이 사라져 보이면 늘림. 피해·폭발 사운드는 폭발 즉시
- [ ] `VisualScale`(현재 0.2)이 생성 FX(`NS_Bomb_Spawn`)에도 적용 — 팩의 `User.Scale Overall` 파라미터로 전달. 폭발 VFX는 영향 없음
- [ ] PIE: 던지기 모션에서 손이 뻗는 순간 날아가는지·던지기 소리 / 폭발 VFX와 본체 사라짐이 맞는지 / 생성 FX 크기가 본체와 어울리는지
- [ ] (2026-09-18 추가) 수류탄 버튼을 누르는 동안 **조준 상태**(조준 FOV·조준 걷기·카메라 방향), 손에서 놓는 순간 조준 해제(원래 조준 중이었으면 유지) / 수류탄 준비~던지기 모션 끝까지 **총 숨김**, 수류탄 든 동안 사격 불가
- [x] **빌드 완료** — 생성 FX **완료 이벤트** 즉시 수류탄 등장·던지기 가능 (`SpawnToProjectileDelay` → **`SpawnFXMaxWait`**(기본 3초, FX가 안 끝날 때 최대 대기)로 대체, 2026-09-18)
- [x] **던지기 몽타주 슬롯 수정** — `AM_WpAttack_Hand_1R_mika_Throw` 슬롯이 `DefaultSlot` → **`DefaultGroup.UpperBody`**로 변경 (ABP가 UpperBody만 읽어 모션이 안 보였음. 준비 몽타주는 정상)
- [x] **던지기는 전신** — ABP 마지막 `Blend Poses by bool`(전신/상체) Active Value = `Is Dashing OR bIsThrowingGrenade`. 슬롯은 UpperBody 그대로(전신 분기가 SlotPose를 통째로 씀)
- [x] ABP 9번은 **EventGraph에서 `Is Aiming` 변수를 세팅하는 곳 한 군데**에 `OR bIsPreparingThrow` 추가 (AnimGraph의 Is Aiming 사용처 여러 곳을 각각 고치지 말 것). 척추 보정 ModifyBone 2개 Alpha = `Is Charging OR bIsPreparingThrow`
- [ ] **생성 FX가 손을 따라가게** — `NS_Bomb_Spawn`은 월드 공간 이미터라 부착해도 입자가 제자리에 남음. 복제(`NS_Bomb_Spawn_Local`) → 각 이미터 Emitter Properties › **Local Space 체크** → `GrenadeData_Throwable.SpawnVFX`에 지정 (C++로는 변경 불가)
- [x] **빌드 완료** — 대시 중 몸을 **펀치 방향에 고정**(카메라 따라 회전 안 함) + `DashPitch`(대시 방향 상하 각도, 위=+) 노출. `MikaData` › Punch › `DashMaxVisualPitch`(기본 60) (2026-09-18)
- [ ] ABP: 대시 중 몸 기울이기 — 척추 보정 노드들과 같은 Component Space 구간에 `Transform (Modify) Bone`(Bone `ValveBiped_Bip01_Pelvis`, Add to Existing, Bone Space, Alpha Bool = `Is Dashing`, Blend In/Out 0.1~0.15), Rotation ← `DashPitch`. 축·부호는 테스트로 결정(ValveBiped 척추는 Roll이 상하였음)
- [x] ABP: 충전 중 고개 숙임 보정 — 충전 전용 척추 보정 노드(Alpha = Is Charging OR bIsPreparingThrow)의 계수를 ABP 변수로(`ChargeSpinePitchScale1/2`, `ChargePitchOffset`), 충전·수류탄 준비별 값은 EventGraph에서 Select로
- [ ] (2026-09-18 추가) 연사 중 탄창이 비면 그 즉시 반동 몽타주 정지 (`APlayerCharacter::OnWeaponShotFired`)

### 수류탄 본체 유지 · 재장전 몽타주 (2026-09-18) — C++ 빌드 완료
- [x] 빌드
- [ ] 수류탄 본체가 폭발 전에 사라지는 원인 = 본체 VFX(`NS_Bomb_Projectile`, 메시 없음)가 재생 시간이 정해져 있어 손에 든 시간+신관(3초) 전에 끝남. 폭발 전이면 끝날 때 재생 재시작하도록 수정 → PIE에서 본체가 폭발까지 보이는지, 재시작 순간 깜빡임이 거슬리는지. 거슬리면 `NS_Bomb_Projectile` 복제 → 시스템 Loop Behavior = Infinite로 바꾼 에셋 사용
- [ ] `ExplosionBodyLingerTime` 10 → **0.1~0.3**으로 되돌리기 (폭발 후 본체 유지 시간. 위 원인과 무관했음)
- [ ] `MikaData` › Animation › **`ReloadMontage`** (UpperBody 슬롯 → 상체만). 장전이 실제로 시작될 때만 재생
- [x] **빌드 완료** — 수류탄은 항상 **조준선 그대로** 발사: 노티파이 순간 수류탄을 조준선 시작점으로 옮기고 같은 속도로 발사. 시작점은 `MikaData` › Grenade › **`GrenadeLaunchOffset`**(카메라 기준 X앞·Y오른쪽·Z위, 기본 (50,0,0) = 기존과 동일)
- [x] **빌드 완료** — 카메라 위아래 제한각: `MikaData` › Camera › **`CameraPitchMin`**(기본 -89.9) / **`CameraPitchMax`**(기본 89.9) → PlayerCameraManager ViewPitchMin/Max (2026-09-18)
- [ ] 위·아래 90° 조준처럼 보이게 — `MikaData` › Camera › **`AimSpinePitchClamp`** 60 → 80~90 (카메라 자체는 엔진 기본 ±89.9°로 이미 가능, 몸 휨만 60°에서 막혀 있음)

### 수류탄 준비 이벤트 · 재장전 길이 · 본체 VFX · 펀치 손 FX (2026-09-18) — C++ 빌드 완료
- [x] 빌드
- [x] 빌드 (펀치 FX 분리·히트 FX·충전 VFX 늘리기)
- [ ] `ChargeHandVFX`에 `NS_Lightning_Cast` 지정 + **`ChargeHandVFXSourceLength`** = 원본 재생 길이(초, 나이아가라 타임라인에서 확인) → `ForcedMaxChargeTime`(2초) 동안 1회 재생되도록 느리게 재생. 0이면 원래 속도
- [x] 빌드 완료 — Punch|FX를 **1 Charge / 2 Dash / 3 Hit** 하위 칸으로 정리. 히트 VFX는 맞은 표면 위치에서 미카 쪽을 향함. **`ChargeHandVFXCameraOffset`**(기본 40cm) — 충전 VFX를 카메라 쪽으로 당겨 몸에 안 가리게. 옆에서 떠 보이면 줄이기
- [ ] `MikaData` › Punch|FX:
  - **`ChargeHandVFX`** — 충전 중 오른손. 충전할수록 **`ChargeHandVFXMinScale`→`ChargeHandVFXMaxScale`**로 커짐 (대시 속도와 같은 충전 비율). 에셋에 `User.Scale Overall`이 있으면 그걸로, 없으면 컴포넌트 스케일로 조절. 루프형 권장
  - **`DashHandVFX`** — 대시 중 오른손 (크기 조절 없음)
  - **`ChargeHandSFX`**(충전 중) / **`DashHandSFX`**(대시 중) — 단계 끝나면 정지
  - **`PunchHitVFX`**(+`PunchHitVFXScale`) / **`PunchHitSFX`** — 대시당 **첫 히트 1번만**. 적·파괴물 = 히트박스 겹침(손 위치), 벽 = 대시 중 캡슐이 막힌 지점(바닥 착지는 제외). 적 캡슐에 먼저 막혀도 히트로 침
- [ ] (재진단) 재시작 코드가 있어도 여전히 폭발 전에 사라짐 — `NS_Bomb_Projectile`이 1초짜리. **사람:** 복제본 만들어 이미터 Loop Behavior = Infinite(또는 본체 파티클 수명을 길게)로 바꾸고 DA `ProjectileVFX`에 지정. 해결되면 **AI:** `AGrenadeBase::OnProjectileVFXFinished`(재시작 코드)와 바인딩 제거
- [ ] 위젯에서 플레이어의 **`OnGrenadeThrowReadyChanged(bReady)`** 바인딩 → true = 던질 수 있음, false = 던짐/취소
- [ ] 장전 몽타주는 이제 무기 DA **`ReloadTime`** 길이에 맞춰 재생 속도가 자동 조절됨 → 무기별로 모션이 너무 빠르거나 느리지 않은지 확인
- [x] `Reload_mika`·`Crouch_Reload_mika` 루트 스케일 트랙 제거 (크기 수정)

### 미카 펀치 개편 — 사거리·판정·넉백·반동·범위 데칼, 랜딩 흡수 (2026-09-18) — C++ 작성, 빌드 대기
원인: 대시가 몽타주 길이(1.47초) 동안 낙하 모드로 감속 없이 날아가(`DashBrakingDeceleration`은 비행 모드용이라 무효) 최대 ~35m를 손 히트박스로 쓸고 감.
- [ ] 빌드 (에디터 닫고)
- 변경:
  - 대시 = 비행 모드 + 제동 0 **일정 속도 직선 이동**. 속도 = 거리 ÷ 대시 시간(몽타주 길이). 끝나면 정지 → 낙하 모드
  - `MinDashSpeed`/`MaxDashSpeed`/`DashBrakingDeceleration` 삭제 → **`PunchMinDistance`(400)/`PunchMaxDistance`(2400)** (MikaData › Punch|Range)
  - 손 박스 `PunchHitbox` 삭제 → 몸 앞 캡슐 **`PunchHitCapsule`** (`PunchHitRadius` 51 / `PunchHitHalfHeight` 132 / `PunchHitForwardOffset` 51, 미카 캡슐의 1.5배). 대시 중에만 켜짐
  - `PunchDamage` 삭제 → **`PunchMinDamage`/`PunchMaxDamage`** (충전 비례). 넉백 **`PunchMin/MaxKnockback`** + `PunchKnockbackUpRatio`, 미카→대상 **방사형** (MikaData › Punch|Hit)
  - 적(폰) 적중 시 대시 중단 + **반동**: `PunchReboundMaxSpeed` × 남은 대시 시간 비율, `PunchReboundUpRatio`. 파괴물은 부수고 계속 진행
  - **미카 랜딩 제거** (`MikaLanding`·다이브·`Landing*` 변수·몽타주 2개) → `PunchSlamMinDownPitch`(30°) 이상 아래로 대시하다 바닥에 닿으면 **`PunchSlamRadius`**(300) 안 전원에 충전 비례 피해·넉백
  - 범위 표시: 충전 중 바닥에 대시 경로 사각형 데칼 (폭 = 판정 지름, 길이 = 충전 비례 거리, 벽에 막히면 거기까지). 머티리얼 **`/Game/V2_HJContents/Materials/Decal/M_PunchRange_Decal`** (AI 생성, 파라미터 Color·FillOpacity·EdgeOpacity·EdgeWidth)
  - 판정에서 자기 소유(손에 든 총·수류탄) 제외 — 히트 FX가 안 뜨던 원인 후보
  - 진단 로그 `[PunchHitFX]` (LogTemp) — 원인 확인 후 제거할 것
- [x] 빌드 (개편 본체)
- [x] 빌드 완료 — `MikaData` › Punch|Range › **`bDebugDrawPunchHit`** 디버그 체크박스 (판정 캡슐: 충전 중 노랑·대시 중 빨강, 착지 공격 반경 1초 주황 구)
- [x] `CLAUDE.md`·`Docs/LevelDesign/Stage.md`에서 미카 랜딩 → 아래 펀치(착지 공격)로 정리, Stage 메트릭스 표 새 펀치 수치로 갱신 (`PROGRESS.md`·`30m_DailyTask.md`·`progress_human.md`·`Docs/BP_AnimSetup_Reference.md`엔 랜딩 언급 남아 있음)
- [x] 빌드 완료 — **정면/측면 적중 분리** (2026-09-18 사용자 상세 명세)
  - 정면(대시 직선에서 옆 거리 ≤ `PunchFrontHalfWidth` 40): 100% 피해, 적은 대시 수평 방향으로, 미카 멈춤 + 대시 거리 × `PunchReboundDistanceRatio`(0.1)만큼 `PunchReboundTime`(0.2초) 동안 반대 방향 반동, 메인 히트 FX
  - 측면: `PunchSideDamageRatio`(0.5) 피해, 적은 미카→적 방향으로 `PunchSideKnockbackRatio`(0.5) 넉백, 미카 계속 진행, 서브 히트 FX(`PunchSideHitVFX`/`Scale`/`SFX`, 대상마다)
  - 넉백 속도 = 대시 거리 × `PunchKnockbackPerDistance`(1.0). `PunchMin/MaxKnockback`·`PunchReboundMaxSpeed/UpRatio` 삭제
  - ~~벽: 메인 히트 FX만~~ → (빌드 완료) 벽도 적 정면처럼 **멈춤 + 반동**. 충돌 콜백은 이동 처리 중이라 `bPendingRebound`로 다음 Tick에 반동 시작 (적 정면도 동일)
- [x] 빌드 완료 (ABP 교체는 남음) — **펀치 후속 4건** (2026-09-18, 계획: `~/.claude/plans/purrfect-stargazing-bonbon.md`)
  - 대시 자연 종료 시 속도 유지: `PunchEndMomentumRatio`(1). 적중·벽·착지 공격은 정지
  - `bIsPunchFullBody` (대시 시작 ~ 펀치 몽타주 끝) → **사람:** ABP EventGraph `Set is Dashing` 입력을 `Get bIsDashing` → `Get bIsPunchFullBody`로 교체 (MCP `add_node`는 변수 지정 불가라 수동). ADR-009 갱신 완료
  - 히트스톱: `PunchHitSlowTimeScale`(0.1) / `PunchHitSlowDuration`(실제 0.08초, 0=끔). 메인 히트만. `EndPlay`에서 복구
  - 메인 히트 FX 충전 비례: `PunchHitVFXScale` → `PunchHitVFXMin/MaxScale`(0.5/1.0), `PunchHitSFXMin/MaxVolume`(0.6/1.0)
- [x] 빌드 완료 — **풀 충전 연출** (2026-09-18)
  - 메인 히트 연출(메인 FX·히트스톱·폭발)은 **풀 충전(충전 ≥ `MaxChargeTime`)** 대시만. 미만이면 정면·벽·착지 공격도 서브 히트 FX (대미지·멈춤·반동은 동일)
  - 메인 히트 폭발: VFX `PunchExplosionVFX`(+`PunchExplosionVFXReferenceRadius`, Punch|FX|3 Main Hit), 반경·피해 `PunchExplosionRadius`(300)/`PunchExplosionDamage`(40) (Punch|Hit). 디버그 켜면 자홍 구
  - 풀 충전 도달: `PunchFullChargeSFX`(2D, Punch|FX|1 Charge) + 이벤트 **`OnPunchFullCharge`** + UI 게이지용 `GetPunchChargeRatio()`
  - 범위 데칼: 풀 충전 전 외곽선만(동적 머티리얼 `FillOpacity` 0) → 풀 충전 후 채움
  - 버그: 바닥 보고 펀치 시 수평(위를 보는) 모션 — 즉시 착지 공격으로 대시가 끝나 `DashPitch`가 바로 0이 됐음 → 몽타주 끝(`EndPunchFullBody`)에서 해제
- [x] 빌드 완료 — **풀 충전 손 VFX · 폭발 범위 데칼 · 데칼 캐릭터 제외 · 반동 딜레이** (2026-09-18)
  - `PunchFullChargeHandVFX`/`Scale` (Punch|FX|1 Charge): 풀 충전 순간 오른손에 부착, 대시 끝에 정지
  - `PunchExplosionDecalMaterial` (Punch|Range) = **`M_PunchExplosion_Decal`** (AI 생성, 주황 원, `FillOpacity`/`EdgeOpacity`/`EdgeWidth`): 경로 끝에 폭발 반경 원. 풀 충전 전 외곽선 → 후 채움
  - `ACharacterBase` 메시·`AWeaponBase` 무기 메시 `bReceivesDecals = false` → 데칼은 벽·바닥에만
  - (빌드 완료) 폭발 반경 충전 비례: `PunchExplosionRadius` → **`PunchExplosionMinRadius`(150) / `PunchExplosionMaxRadius`(300)**. 범위 원 데칼이 충전하며 커짐, 실제 폭발은 풀 충전에서만이라 항상 Max
  - 반동 딜레이 `PunchReboundDelayFull`(0.1) / `PunchReboundDelayNormal`(0.05) (Punch|Hit, 게임 초 — 히트스톱 중엔 길게 느껴짐). 딜레이 동안 제자리(비행 모드)
- [x] 빌드 완료 — **폭발 상시화 · 폭발 넉백 · 조준 허리 틀기 · 바닥 목표 착지 공격** (2026-09-18)
  - 폭발은 풀 충전 아니어도 정면·벽·착지 공격마다 (메인 FX·히트스톱은 풀 충전만). 새 카테고리 **Punch|Explosion**: `PunchExplosionMin/MaxRadius`(150/300), **`PunchExplosionMin/MaxDamage`**(10/40, 펀치 피해와 분리), **`PunchExplosionKnockbackRatio`**(0.25 = 이번 대시 펀치 넉백의 25%, 폭발 중심 바깥쪽). `PunchExplosionDamage` 삭제
  - `M_PunchExplosion_Decal` 색 주황 → 분홍 (1, 0.35, 0.8) — 완료
  - 조준 허리 틀기: `MikaData` › Camera|AimWaist › `AimWaistYawOffset`(15°) / `AimWaistBlendSpeed`(10) → 미카 `AimWaistYaw`(BlueprintReadOnly, 조준 중 보간). **사람:** ABP EventGraph에 `Get AimWaistYaw` → 새 ABP float 변수 Set, AnimGraph 척추 보정 뒤·Component To Local 앞에 ModifyBone(Spine1, Add to Existing, Bone Space) 추가 → 좌우 틀기 축(X/Y/Z) PIE로 찾기
  - **바닥 목표 착지 공격**: 원이 바닥에 떴는데 미끄러져 앞으로 가던 문제 → 범위 원과 대시가 같은 트레이스(`TracePunchPath`) 사용. 경로 끝이 걸을 수 있는 바닥이면 그 지점까지 가서 착지 공격(지상 출발은 수평 이동, 공중은 바닥에 닿는 순간). → (빌드 완료) 평지 원이 너무 잦아 **`PunchSlamMinDownPitch`(30°) 부활**: 30° 이상 아래 + 바닥 = 착지 공격, 지상에서 얕게 아래 = 수평 대시. 폭발 원은 실제 폭발 지점(바닥 목표·벽)에만, 허공 끝이면 숨김. 벽 예측은 가는 선 → **미카 캡슐 스윕**(이동과 같은 채널·응답, 반높이 −10cm)으로 교체
  - (빌드 완료) 조준 카메라 오프셋: `MikaData` › Camera|Aim › `AimSocketOffsetRight`(20) / `AimSocketOffsetUp`(15) → 조준 중 스프링암 SocketOffset Y·Z에 더함 (보간 = `CoverPeekInterpSpeed`). 엄폐 좌우 이동은 이 위치 기준
  - (빌드 완료) 반동: 등속 이동 후 0으로 끊던 것 → 딜레이 후 **초기 속도만 주고 낙하 모드**(제동·중력으로 자연 감속). 초기 속도 = 대시 거리 × `PunchReboundDistanceRatio` ÷ `PunchReboundTime`
  - ~~발견: `MikaData.DefaultSocketOffsetY`가 안 먹음~~ → (빌드 완료) `APlayerCharacter::ApplyDefaultSocketOffset()` 추가, 미카가 DA 복사 후 다시 호출
  - (빌드 완료) 폭발 원 ↔ 실제 폭발 어긋남 수정: 공중에서 아래로 칠 때 원은 몸 중심선이 바닥에 닿는 곳, 폭발은 발이 닿는 곳이라 30°에서 ~1.5m 차이 → 지상 가파른 아래만 선, **나머지는 캡슐 스윕 접촉점**. 원을 닿는 면에 수직 투사(벽면 정면, 적이면 바닥)
  - 남은 차이 가능성: ① 폭발 VFX 크기는 `PunchExplosionVFXReferenceRadius`(300, 추정값)에 따라 달라 원과 안 맞을 수 있음 ② 폭발 피해는 적 캡슐이 반경에 **조금이라도** 걸리면 들어가 원 밖 ~34cm까지 맞음
- [ ] **사람:** `MikaData` › Punch|Range › **`PunchRangeDecalMaterial`** = `M_PunchRange_Decal`, **`PunchExplosionDecalMaterial`** = `M_PunchExplosion_Decal`
- [ ] **사람:** PIE — 사거리 4~24m, 적 적중 시 반동, 넉백·대미지 비례, 아래 대시 착지 공격, 데칼 표시, 히트 FX (안 뜨면 Output Log에서 `[PunchHitFX]` 확인)
- [ ] **AI:** 히트 FX 원인 확인되면 진단 로그 제거

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


### 바로 다음에 할 일 — 미카 모션 연결 (2026-09-18, 사람이 몽타주 만든 뒤)
- [x] 빌드 완료 (2026-09-21) — 폭발 원 캡슐 예측·면 투사, `ApplyDefaultSocketOffset`
- [ ] 반동 모션 `Jmp_BackAir`: `MikaData`에 `PunchReboundMontage` 추가 → `BeginReboundMove`에서 재생 (반동 중 전신: `bIsPunchFullBody` 유지 or 별도 조건)
- [ ] 점프 `Jump_Up_B`/`Jump_Down_B`: ABP 점프·낙하 상태 (C++ 불필요할 가능성, `Is Falling`·Velocity.Z)
- [ ] 착지 공격 모션 `Land_Spawn_Wait`: `MikaData`에 `PunchSlamMontage` 추가 → `PunchSlam`에서 대시 몽타주 대신 재생
- [ ] 고지대 착지 모션 `Land_Base_Wait`: 기존 하드 랜딩(`HardLandingSpeedThreshold`, `OnLanding`) 흐름에 연결

### 나중에 할 일 (2026-09-18 사용자 요청) — 사람 쪽 목록은 Todo_Human.md › 나중에 할 일
- [ ] 수류탄 던질 수 있는지 UI — `APlayerCharacter::OnGrenadeThrowReadyChanged(bReady)` 바인딩하는 위젯 (C++ 이벤트는 이미 있음)
- [ ] 펀치 충전 UI — 풀 충전 알림 `AMikaCharacter::OnPunchFullCharge` 바인딩 + 충전 게이지 `GetPunchChargeRatio()`(0~1) (C++ 이미 있음)
- [x] 펀치 충전 중 점점 커지는 카메라 흔들림 — (빌드 완료) `MikaData` › Punch|Camera › `ChargeShakeMaxLocation`(2cm) / `ChargeShakeMaxRotation`(0.5°) / `ChargeShakeFrequency`(20Hz). 카메라 컴포넌트 상대 위치·회전에 펄린 노이즈, 감쇠·지연 없음, 떼면 즉시 복귀 (스프링암 카메라 랙은 원래 꺼져 있음)
- [ ] 펀치 **풀 충전** 후 대시에만 나오는 VFX·SFX

### 보류 (필요해지면)
- [ ] 적 조준(Ironsights) 모션 — `bIsAiming` + `BTService_SetAiming` + ABP `Blend Poses by bool`·`Aim_Space_Ironsights`. 2026-09-17 "적은 디테일 불필요"로 보류
- [ ] 미카 약/강 펀치 분기 — `MikaData`에 `PunchLightMontage`(`Attack_Hand_1R_Mika`)·`PunchPowerChargeRatio` 추가, 충전 비율 기준 미만이면 약펀치 몽타주. 2026-09-17 "충전·강펀치만 먼저"로 보류
- [ ] EQS 대신 C++ 엄폐 태스크(`Find Cover Location` / `Is In Cover`) — EQS 판정 문제가 계속되면 전환 검토

### 미카 점프 ABP (2026-09-21) — 진행 중
- 애니: `MikaJump/` 에 `Jmp_Base_B_mika`(30f, 0~3 도약 / 3~7 상승 / 7~16 하강 / 16~30 착지), `Jump_Down_B_Loop_mika`(12f 반복), `Land_Spawn_Wait_mika`(31f), `Jmp_BackAir_mika`(37f). 모두 루트 스케일 트랙 제거 완료
- ABP 구조: 지상 로코모션 ↔ `AirLoco` 상태 머신을 **`Blend Poses by bool`**(Active = `isInAirPose`)로 전환 → `LocoPose` 캐시.
  ※ 처음엔 `GroundPose` 캐시를 상태 머신 `Ground` 상태 안에서 참조했으나 **PIE에서 T포즈** → 상태 머신 밖 블렌드 방식으로 변경
- `isInAirPose` = `Is Falling` OR `LandBlendTimer > 0` (착지 후 0.47초 유지, EventGraph에서 계산)
- [ ] 남은 작업(사람): 컴포짓 4개 Loop 해제, 전환 Duration 0.05~0.1로 낮추기(반응 지연), 절벽 낙하 시 `FallLoop` 연결 확인
- [ ] 남은 작업(AI): `Land_Spawn_Wait_mika` 몽타주 → `PunchSlamMontage`(착지 공격) / 고지대 착지, `Jmp_BackAir_mika` 몽타주 → `PunchReboundMontage`(반동) — 사람이 몽타주 만든 뒤 연결
- [x] 빌드 완료 (2026-09-22) — `bIsInAirPose`(낙하 중 OR 착지 후 `LandPoseHoldTime` 이내)를 `APlayerCharacter`에서 계산해 노출. `MikaData` › Fall › **`LandPoseHoldTime`**(0.47). **사람:** ABP EventGraph의 Branch·LandBlendTimer·Max·비교 노드 삭제 → `Get bIsInAirPose` → `Set isInAirPose` 한 줄로 교체
- [x] (빌드 완료 2026-09-22) 점프 ABP 보조 값 — `bIsLandingSoon`(발밑 캡슐 스윕으로 착지 예측, `MikaData` › Fall › `LandAnticipationTime` 0.15), `AirPoseAlpha`(0~1 보간, `AirPoseBlendSpeed` 12). **사람:** `Blend Poses by bool` → **Layered blend per bone**(Base = 지상, Blend 0 = AirLoco, Blend Weights 0 = `AirPoseAlpha`, Layer Setup: `valvebiped_bip01_pelvis` 0 / `valvebiped_bip01_spine` -1), `FallLoop·JumpDown → Land` 조건에 `bIsLandingSoon` OR 추가
- [x] (빌드 완료) 충전 중 좌우 이동이 앞걸음으로 보이던 버그 — 충전 시작 시 `bOrientRotationToMovement = false`(조준과 동일), 미발동 종료 시 복구
- [x] (빌드 완료) 착지 모션은 전신 — `LandPoseAlpha`(착지 예측~착지 유지 시간 동안 1) 추가. **사람:** ABP에서 `AirLoco` 출력을 `AirPose`로 캐시 → ① Layered blend per bone(하체만, W=`AirPoseAlpha`) → ② `Blend`(A = ①, B = `AirPose` 전신, Alpha = `LandPoseAlpha`) → `LocoPose`
- [x] (빌드 완료 2026-09-22) 착지 늦게 나오는 문제 — 예측 거리에 중력 가속 포함(`v·t + ½·g·t²`), `LandPoseAlpha`는 켜질 때 즉시 1·꺼질 때만 보간
- [x] (빌드 완료 2026-09-22) 착지 지연 2차 — 메시 Tick을 캐릭터 Tick 뒤로(ABP가 LandPoseAlpha를 1프레임 늦게 읽던 문제), ABP Land 상태 시퀀스 플레이어 StartPosition 0.05(=Jmp_Base_B 0.583초, 발 닿기 직전부터), MikaData `LandAnticipationTime` 0.15→0.08(크로스페이드와 맞춤)
- [x] (2026-09-22) 착지 2번 재생 — 원인: 예측으로 공중에서 Land 진입 → `Land→JumpStart`(Is Falling) → `→Land` 핑퐁. 조치: `*→Land` 조건 `Land Pose Alpha > 0.99`(점프 직후 감쇠 중 재진입 방지), `LandAnticipationTime` 0(예측 끔). **예측 다시 켜려면** `Land→JumpStart` 조건을 `Is Falling AND NOT 착지예측`으로 바꿔야 함 (ABP 변수 추가 필요)
- [x] (2026-09-22) 착지 반복 진짜 원인 — ABP Land 상태 시퀀스 플레이어 **Loop Animation이 켜져 있었음** (클립 0.42초 < 유지 0.47초+페이드 → 다시 재생). Loop 끔. PIE 확인: 착지 프레임에 Land Pose Alpha 즉시 1 (1프레임 지연 해결)
- [x] (빌드 완료 2026-09-22) `JumpAnimPlayRate`(MikaData › Fall, 현재 0.8) — ABP AirLoco 시퀀스 플레이어 5개 PlayRate를 Property Access `Character.JumpAnimPlayRate`에 바인딩. 착지 유지 시간 = `LandPoseHoldTime ÷ JumpAnimPlayRate`
- [x] (빌드 완료 2026-09-23) 착지 길이 자동화 — `LandAnimation`(AC_Land_mika)·`LandAnimStartTime`(0.05) DA 추가. `GetLandHoldTime()` = (클립 길이 - 시작 지점) ÷ 재생 속도. ABP Land 시퀀스 플레이어 StartPosition도 `Character.LandAnimStartTime`에 바인딩 → 클립 길이를 바꿔도 C++ 수정 불필요
- [x] (빌드 완료 2026-09-23) `bCameraFollowMesh` — 공중 포즈일 때 스프링암 TargetOffset.Z를 (기준 본 높이 - 평상시 높이차)만큼 올림. `CameraFollowMeshBone`·`InterpSpeed`·`MaxOffset` DA. 앉기 보정과 합산(`CrouchCameraZ + CameraFollowMeshZ`)
- 주의: MikaData 값은 BeginPlay에서 복사 — **PIE 실행 중 DA를 바꾸면 PIE 재시작해야 적용**
- [ ] (진행 중) `JumpAnimPlayRate` 바인딩 의심 — PIE에서 캐릭터 값은 정상(0.1, LandAnimation 세팅됨) 확인. ABP Property Access 바인딩이 실제로 먹는지 불명. 구분 실험: 노드 PlayRate 기본값 0.6 + DA 0.1 → 매우 느림=바인딩 동작 / 적당히 느림=기본값만 동작(바인딩 실패, ABP 변수 배선으로 교체 필요) / 변화 없음=AirLoco가 아닌 다른 경로
  - 1차 결과(0.6): 변화 없음 → 0.15로 재실험. DA 재생 속도는 1로 복구(착지 유지 시간이 ÷배율이라 0.1이면 4초간 착지 포즈에 갇힘)
  - **결론(2026-09-23): Property Access 바인딩은 먹지 않음. 원인은 시퀀스 플레이어의 PlayRate가 핀으로 노출돼 있지 않았던 것.** 조치: 노드 `ShowPinForProperties`에서 PlayRate 노출(Python) → ABP 변수 `JumpPlayRate` 추가 → 이벤트 그래프에서 `Set Land Pose Alpha` 뒤에 `Set JumpPlayRate ← Character.JumpAnimPlayRate` 연결 → 5개 상태의 PlayRate 핀에 변수 Get 배선. **ABP 핀 바인딩이 안 먹으면 핀 노출부터 확인할 것**
- [x] (빌드 완료 2026-09-23) 장전 중 왼손 부착 — 미카 스켈레톤에 `LeftHand` 소켓 추가(L_Hand 본). 장전 시작 시 무기를 그 소켓에 붙이고 무기의 `LeftHandGrip` 소켓이 손에 오도록 역보정, `ReloadTime` 뒤 오른손 복귀. DA: `bAttachWeaponToLeftHandOnReload`, `ReloadLeftHandSocket`
- [x] (빌드 완료 2026-09-23) 총성 소리 지각 — `AWeaponBase::Fire`에서 `UAISense_Hearing::ReportNoiseEvent`(태그 Gunshot). 무기 DA: `FireNoiseRange`(3000), `FireNoiseLoudness`(1). **발사할 때만** 소음, 발소리·이동 소음 없음. **사람:** 적 AI Perception에 Hearing 감각이 켜져 있어야 들린다
- [x] (빌드 완료 2026-09-23) BGM 뼈대 — `Source/Audio/BGMDataAsset.h`(Explore·Combat·Boss 곡, 페이드, `CombatExitDelay`), `BGMSubsystem`(UWorldSubsystem: SetBGMData/StartBGM/SetState/NotifyCombat/StopBGM). 에셋 `/Game/V2_HJContents/Data/BGMData` 생성. **사람:** 곡 넣고 레벨 BP에서 SetBGMData→StartBGM, 전투 지점에서 NotifyCombat 호출. **AI 나중:** 피격·발사·적 발견에서 자동 NotifyCombat 연결
- [x] (빌드 완료 2026-09-23) 펀치 반동 몽타주 — MikaData `PunchReboundMontage`, `BeginReboundMove()`에서 재생. **사람:** 구르기 몽타주 만들어 DA에 지정
- [x] (2026-09-23) 카와이 피직스 — 마켓 바이너리(5.6)라 경고가 났음. `KawaiiPhysics.uplugin` EngineVersion 5.7.0·Installed false로 바꾸고 Binaries/Intermediate 삭제 후 소스 빌드. v1.20.0은 UE5.3~5.7 지원. 미카 ABP AnimGraph 꼬리(ModifyBone Pelvis → **카와이 16개** → ComponentToLocalSpace)에 치마 8(Skirt_F/F_R/R/B_R/B/F_L/L/B_L) + 머리 8(Hair_F/L/R/B_0/B_2/B_5/B_7/B_9) 추가. 설정은 미유와 동일(Damping 1, WorldDampingLocation 2, WorldDampingRotation 0.8, Stiffness 1, Radius 1). **사람:** 세부 조정
- [x] (빌드 완료 2026-09-23) 장전 중 총 크기 튐 버그 — 원인: 왼손 부착 시 `GripLocal.Inverse()`에 **메시 배율까지 포함**돼 총이 작아졌다 커졌음. 조치: 역보정에서 배율 제거(`SetScale3D(1)`), 장전 전 상대 트랜스폼(`PreReloadWeaponTransform`)을 저장해 복귀 시 그대로 복원
- [x] (빌드 완료 2026-09-23) 반동 몽타주 재생 속도 — MikaData `PunchReboundMontagePlayRate`
- [x] (빌드 완료 2026-09-23) BGM 볼륨 — `UBGMSubsystem::SetVolume(볼륨, 페이드)` / `GetVolume()`. 지정 안 하면 DA의 Volume 사용
- [x] (빌드 완료 2026-09-23) 적 교전 조준 대기 — `BTTask_FireAtTarget`에 `AimDelay`(1.0) · `ReengageGap`(3.0). 새 교전이면 타겟을 바라보고(SetFocus) 1초 겨눈 뒤 사격. 연사 중에는 대기 없음. `AEnemyCharacter::GetLastFireTime()` 추가
- [ ] (진단 중) 총성 청각 — `EnemyAIController::OnTargetPerceptionUpdated`에 `[AIPerception]` 로그 추가. 팀(플레이어 0 / 적 1)·HearingRange 1200·FireNoiseRange 3000은 정상. 로그에 Hearing이 안 찍히면 적 컨트롤러가 C++(AEnemyAIController)가 아닐 가능성 확인
- [x] (2026-09-23) 카와이 리밋 — `DA_KawaiiLimits_MikaSkirt`(골반 r11·양 허벅지 r8) / `DA_KawaiiLimits_MikaHair`(머리 r10·목 r6·상체 r12), 전부 Outer. 16개 노드에 자동 배정(치마 8 / 머리 8). **임시값이라 사람이 조정**
- [x] (빌드 완료 2026-09-23) ABP 컴파일 경고 `__FloatProperty_592` — 원인: Land 시퀀스 플레이어 StartPosition의 **Property Access 바인딩(`Character.LandAnimStartTime`)이 해석 실패**. PlayRate와 같은 문제. 조치: 바인딩 제거 → ABP 변수 `LandStartPosition`(이벤트 그래프에서 Character.LandAnimStartTime 복사) → 핀 노출 후 배선. **이 ABP에서 Character 경유 Property Access는 쓰지 말 것**
- [x] (빌드 완료 2026-09-23) 장전 총 크기·위치 2차 수정 — 상대 좌표 계산은 캐릭터 메시 배율(≠1) 때문에 단위가 어긋남. **월드 기준**으로 `GripInActor.Inverse() * HandWorld` 계산 후 `SetActorTransform`, 배율은 기존 값 유지
- [x] (빌드 완료 2026-09-23) 적이 감지해도 안 돌아보는 문제 — 감지 시 `SetFocus(Actor)` + `AEnemyCharacter::SetFaceTargetMode(true)`(컨트롤러 Yaw 사용·이동 방향 회전 끔). `ForgetTarget()`에서 `ClearFocus` + 원복
- [x] (빌드 완료 2026-09-23) 반동 몽타주 전신 — 재생과 동시에 `bIsPunchFullBody` 켜고 몽타주 실제 길이 뒤 `EndPunchFullBody` 타이머
- [x] (빌드 완료 2026-09-23) 착지 재생 속도 분리 — DA `LandAnimPlayRate`(0.6) / `JumpAnimPlayRate`(0.1). ABP 변수 `LandPlayRate`를 Land 노드 PlayRate에 배선. `GetLandHoldTime()`은 LandAnimPlayRate 기준
- [x] (빌드 완료 2026-09-23) 반동 백덤블링 착지 어긋남 — `AMikaCharacter::Landed()`에서 반동 몽타주 재생 중이면 `StopAnimMontage` + 전신 해제 → 몽타주의 자체 착지는 버리고 공용 착지 모션이 이어받음
- [x] (빌드 완료 2026-09-23) 무기 디버그 표시 — DA `bDebugWeaponTrail`. `DebugWeaponTrailInterval`(10프레임)마다 총 루트(구)·LeftHandGrip(자홍 구)·둘 사이 선, 이전 위치와 잇는 선, 배율·손까지 거리 텍스트. 왼손 부착 중이면 노랑
- [x] (빌드 완료 2026-09-23) 적이 멈추는 문제 — 조준 진행을 폰이 기억(`UpdateAimReady`/`ClearAimProgress`). BT 태스크가 중간에 끊겨도 1초를 처음부터 다시 세지 않음
- [x] (빌드 완료 2026-09-23) 전투 BGM 자동 — `EnemyAIController::UpdateCombatMusic()`이 경계+타겟 보유 상태에서 1초마다 `NotifyCombat()`. 보스(`AEnemyCharacter::bIsBoss`)는 제외. **사람:** 보스 BP에 bIsBoss 체크
- [x] (2026-09-23) 카와이 중력 — 16개 노드 `UseWorldSpaceGravity` 켜고 `Gravity` Z 치마 -2.0 / 머리 -1.2, Damping 0.6, Stiffness 치마 0.35·머리 0.25, Radius 치마 3·머리 2. 치마 리밋 구 7개로 확장(골반 14, 허벅지 위·아래 10/9, 종아리 8)
- [x] (빌드 완료 2026-09-23) 장전 총 43m 날아감 — 원인: 손 소켓 월드 트랜스폼의 배율(메시 150%)이 이동량에 한 번 더 곱해짐. 조치: 손·총·그립 트랜스폼에서 배율을 모두 제거하고 위치·회전만으로 정렬, 마지막에 총 배율만 복원
- [x] (2026-09-23) 점프 블렌드 2배 — AirLoco 전환 크로스페이드 8개 0.08→0.16, `AirPoseBlendSpeed` 12→6
- [x] (2026-09-23) **카와이 충돌 구 단위 버그** — 이 스켈레톤은 본 컴포넌트 스케일이 **100**이라 `OffsetLocation`은 **미터 단위**(0.09 = 9cm). 처음에 4·12·22로 넣어 구가 4~22m 밖에 있었고 무릎이 전혀 안 막혔음. 반경(Radius)은 컴포넌트 cm 그대로. 치마 구 13개(골반 + 좌우 허벅지 3·종아리 3)로 재배치
- 참고: 카와이 본 제약(Bone Constraints)은 **같은 노드 안의 본끼리만** 연결 가능. 치마 8가닥이 각각 별도 노드라 가닥끼리 묶을 수 없음. 가닥을 면처럼 묶으려면 치마 본들이 공통 부모 하나 아래로 묶인 리그가 필요 (지금은 충돌 + LimitAngle 55도로 대응)
- [x] (빌드 완료 2026-09-24) 스킬 쿨타임 아이콘 — `USkillIconWidget`(UI/)가 아이콘·시계방향 덮개·번쩍임·숫자를 코드로 조립. `SkillType`(Punch/Grenade/Custom)으로 자동 갱신. 펀치는 `GetPunchCooldownRemaining()/GetPunchCooldownDuration()`(신규), 수류탄은 `GetGrenadeCount()`로 개수 표시 후 0이면 어둡게. 쿨타임 종료 시 `FlashDuration`(0.25초) 동안 번쩍임
  - 머티리얼 `M_UI_RadialCooldown` — UI 도메인·Translucent. Custom HLSL이 `atan2`로 12시 기준 시계방향 각도를 구해 `Percent`와 비교. 파라미터: `Percent`(0~1), `SweepColor`. 위젯이 MID로 매 프레임 갱신
  - 에셋: `WBP_SkillIcon_Punch`(라벨 우클릭), `WBP_SkillIcon_Grenade`(라벨 G). HUD 우하단 배치. **사람:** `Icon Texture` 지정 필요
- [x] (빌드 완료 2026-09-24) 히트마커 C++ 연결 — `UCrosshairWidget`(UI/)이 기존 WBP 애니(`Anim_Hit`·`Anim_HeadShot`·`Anim_Killed`)를 재생. `WBP_Crosshair_V2` 부모를 이 클래스로 변경
  - `APlayerCharacter::OnHitConfirmed(bool bHeadshot)` / `OnEnemyKilled` 델리게이트 추가, `AWeaponBase::ReportHitToPlayer()`가 히트스캔 명중 시 호출. 헤드샷 판정은 `Hit.BoneName`에 `HeadBoneKeyword`("head") 포함 여부. 맞은 대상이 이미 죽었으면 처치 마커
  - 주의: BP 그래프가 애니 변수를 읽고 있어 C++ 애니 프로퍼티에 `BlueprintReadOnly` 필요했음
- [x] (빌드 완료 2026-09-23) 플레이어 체력바 오버워치2 스타일 — `UPlayerHealthBarWidget`(UI/)가 칸을 **코드로 자동 생성**. 기본 10칸, 칸 하나 = 최대 체력 ÷ 칸 수. 각 칸은 작은 ProgressBar(배경=빈 칸, 채움=흰색)이고 `RefreshSegments()`가 `(현재체력 - 칸시작)/칸당체력`을 0~1로 잘라 칸마다 채움. `ACharacterBase::OnHealthChanged`에 자동 연결(`bAutoBindToPlayer`). 조절값: SegmentCount·SegmentSize·SegmentGap·CornerRadius·FillColor·EmptyColor·OutlineColor/Width
  - 에셋: `WBP_PlayerHealthBar`(부모 = PlayerHealthBarWidget), `WBP_Crosshair_V2`의 HPBar 안에 배치. 옛 ProgressBar와 HP_Segments 묶음은 제거
  - Build.cs에 `Slate`·`SlateCore` 추가 (FSlateBrush/FProgressBarStyle 링크)
  - **사람:** 잘못 생성된 `/Game/V2_HJContents/V2UI` 에셋 삭제 필요 (AI는 에셋 삭제 권한 없음)
- [ ] (빌드 대기 2026-09-23) 대미지 플로터 — `WBP_DamageNumber` 텍스트 빨강→**흰색**, 폰트 외곽선 2px 추가. `ADamageNumberActor`에 포물선 연출: `bArcMotion`(기본 true), `ArcUpSpeed`(260), `ArcSideSpeed`(150, 좌우 무작위), `ArcSpeedVariance`(0.3), `ArcGravity`(-800). 좌우 방향은 **카메라 기준**이라 어느 각도에서 봐도 화면에서 옆으로 튐. **참고:** WBP의 `Rise` 애니에 2D 트랜스폼 트랙이 남아 있어 위로 뜨는 움직임이 중복될 수 있음 — 필요하면 그 트랙만 제거
- [x] (빌드 완료 2026-09-23) 점프 중 달리기가 공중 모션에 섞이는 문제 — `LocoGroundSpeed`(공중이면 0) 추가, ABP 이벤트 그래프의 `Set GroundSpeed` 입력을 `Character.LocoGroundSpeed`로 교체. DA 토글 `bFreezeLocoSpeedInAir`(기본 true)
- 주의: 애님 그래프 포즈 출력은 **한 곳에만** 연결된다. 다른 노드에 이으면 기존 연결이 조용히 끊긴다 (T포즈 원인). A/B 테스트 후 반드시 원래 연결을 복구할 것
- [x] (2026-09-23) **프레임 드랍 원인 규명** — 게임 스레드가 TG_PostPhysics에서 애니 병렬 평가를 기다리며 정체. 원인은 **카와이 충돌 구가 매 프레임 누적**된 것.
  - 카와이는 `#if WITH_EDITOR`에서 매 평가마다 `ApplyLimitsDataAsset()` 호출 → `SourceType == DataAsset`인 항목만 지우고 데이터 에셋 내용을 다시 Append.
  - 그런데 `SourceType = DataAsset`은 **에디터 디테일 패널에서 사람이 편집할 때(PostEditChangeChainProperty)만** 설정됨. **스크립트(Python)로 만든 리밋은 이 표시가 없어 지워지지 않고 무한 누적** → 프레임당 +13개.
  - 측정: 충돌 13개 = 2.1ms→8.3ms(60초), 충돌 5개 = 1.2→3.4ms, 충돌 없음 = 0.8ms, 카와이 분리 = 1.1ms.
  - **조치: 리밋을 데이터 에셋 대신 각 노드의 `SphericalLimits` 배열에 직접 넣음** (노드 리밋은 이 경로를 타지 않음). 결과 65초 시점 1.15ms / 애니 0.15ms.
  - `SourceType`은 읽기 전용이라 Python으로 못 고침. **앞으로 카와이 리밋을 스크립트로 만들 땐 데이터 에셋 대신 노드에 직접 넣을 것.**
  - 참고: 카와이는 노드 Alpha가 0이어도 시뮬레이션을 계속 돌린다 (A/B 테스트 시 그래프 연결을 끊어야 함)
- [x] (2026-09-23) 점프 골반 높이 축소 — `Jmp_Base_B_mika` 골반 Z 트랙에서 기준 높이(0.6626) 위 성분을 40%로 축소. 정점 +94cm → +37cm. 착지 웅크림(0.39)은 유지. 원본 백업: `Jmp_Base_B_mika_Backup`
- [ ] (빌드 대기) 착지 포즈 중단 — `IsLandPoseInterrupted()`(이동 입력·조준, 미카는 충전·대시·펀치 전신·수류탄 추가) 참이면 착지 유지 즉시 종료. 착지 모션 끝에서 멈춰 다른 행동이 안 되던 문제
- [ ] 점프 중 펀치 카메라 = A안 확정(메시 쪽 수정). `Jmp_Base_B_mika`의 골반 Z 트랙에 점프 높이가 구워져 있음 → 본 트랙 키 수정(복제본 백업 후)으로 캡슐과의 높이 차 줄이기. `bCameraFollowMesh`(B안)는 코드에 남아 있고 기본 끔으로 두면 됨
- [x] (빌드 완료) 충전 중 카메라 아래 각도 확장 — `MikaData` › Camera › **`ChargeCameraPitchMin`**(-89.9). 충전 시작 시 적용, 끝나면 `CameraPitchMin`으로 복귀
