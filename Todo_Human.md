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

## 3. `[ ]` 앉기(Crouch) 비주얼 — ABP 작업만 남음

**C++/입력은 이미 완성**돼 있음 (조사 완료):
- `bCanCrouch=true`, `bCrouchMaintainsBaseLocation=true` (PlayerCharacter.cpp 생성자)
- `Crouch` 입력 → `StartCrouch()`=`Crouch()` / `StopCrouch()`=`UnCrouch()` (PlayerCharacter.cpp:411~412)
- → C키 누르면 **캡슐이 줄고 카메라가 내려옴 = 실제 앉기 작동 중**. 메시가 서 있는 건 ABP에 앉기 포즈가 없어서일 뿐.

비주얼로 보이게 하려면 (ABP_Mika에서):
- [ ] 이동 상태에서 `Get Is Crouched`(Movement Component) 또는 캐릭터의 crouch 여부를 bool로 읽기
- [ ] 그 bool로 **Idle/Walk ↔ CrouchIdle/CrouchWalk** 블렌드(상태머신 분기 또는 Blendspace)
- [ ] 앉기용 애님(CrouchIdle, CrouchWalk) 에셋 확보 후 연결
- [ ] 앉아서 사격: 상체 사격 몽타주는 그대로 UpperBody 슬롯에 재생되므로 하체만 crouch면 OK — 확인
- [ ] 앉아서 턴: turn-in-place도 crouch 포즈 버전 필요하면 추가 (없으면 일반 turn 재생)

> 요약: **코드 추가 불필요, ABP에 앉기 포즈/상태 분기만 추가하면 됨.**

---

## 4. `[ ]` 캐릭터 DA에 반동(발사) 모션 할당

발사 몽타주를 무기별 → **캐릭터 공통**으로 옮김 (`MikaDataAsset.FireMontage`).

- [ ] `DA_Mika`(UMikaDataAsset)의 **Animation > FireMontage** 슬롯에 발사(반동) 몽타주 할당
- [ ] 모든 무기 공통으로 적용되는지 확인 (StartFire 시 캐릭터 FireMontage 재생)

> C++ 완료. 무기 DA의 FireMontage는 제거됨 — 이제 캐릭터 DA에서만 설정.

---

## 5. `[ ]` 적 캐릭터 추가

기본 적 + 아키라(보스) 등 적 캐릭터 제작 시작.

- [ ] 적 BP 자식 클래스 생성 (AEnemyCharacter 계열: AR/Shotgun/Sniper/MG/Shield/LargeSweeper 등)
- [ ] 적 메시·ABP·DataAsset 연결
- [ ] AIController / BehaviorTree / Blackboard 연결
- [ ] 테스트 레벨에 배치해 전투 확인

> 상세 목록·BT·BB 키는 [progress_human.md](progress_human.md) §1·§7·§8 참고. 우선 기본 적 1종(AR)부터.
