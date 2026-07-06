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
