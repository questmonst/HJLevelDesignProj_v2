# Todo_Human (근시일 수동 작업)

> 사람이 에디터/플레이로 직접 해야 하는 단기 작업 목록.
> 전체 수동 작업 레퍼런스는 [progress_human.md](progress_human.md) 참고.
> 완료 시 `[x]` 체크.

---

## 1. `[ ]` 유탄 발사기 테스트

GL(유탄 발사기) 발사 체인 동작 확인.

체인: `BP_Weapon_GL → WeaponData_GL(GrenadeClass=BP_Grenade_Launcher) → BP_Grenade_Launcher → GrenadeData_Launcher`

- [ ] GL 들고 발사 시 유탄이 머즐에서 조준 방향으로 날아가는지
- [ ] 발사 0.5초(ArmingDelay) 후 바닥·벽 충돌 시 폭발하는지 (그 전엔 튕김)
- [ ] 폭발 시 범위 데미지(반경 300)가 주변 적에게 들어가는지
- [ ] NS_Bomb VFX 3종 표시: 생성(Spawn) / 비행 중 본체(Projectile, 0.5배) / 폭발(Explosion, 반경에 맞춰 스케일)
- [ ] 탄도 확인 — 현재 발사 속도 8000은 빠르고 평평함. 포물선 원하면 `WeaponData_GL.ProjectileSpeedOverride`를 2000~3500으로 낮추기 (`bShowArcTrajectory` 미리보기와 같은 값)

> 값 조정은 `GrenadeData_Launcher`(폭발/VFX/크기) 와 `WeaponData_GL`(발사 속도/탄약)에서.

---

## 2. `[ ]` 수류탄 손에 부착 후 던지기 (투척 흐름 개선)

현재 투척 수류탄은 **던지는 순간(Release)에만 스폰**됨 — 손에 들고 있는 단계가 없음. (이전에 애니메이션 미제작이라 보류한 부분.)

목표 흐름:
- [ ] 조준 시작(`StartGrenadeThrow`) 시 수류탄 스폰 → 손 소켓에 부착, `NS_Bomb_Spawn` 재생 후 `NS_Bomb_Projectile` 들고 대기
- [ ] 던지기(`ReleaseGrenadeThrow`) 시 손에서 떼어내 발사(Launch)
- [ ] 던지기 애니메이션(몽타주) 제작 후 연결 — 들기/던지기 모션
- [ ] 손에 든 동안 궤도 미리보기(`bShowArcTrajectory`)와 자연스럽게 연동

> 관련 코드: `PlayerCharacter_Grenade.cpp` (StartGrenadeThrow / ReleaseGrenadeThrow). 흐름 변경은 C++ 수정 필요 — 애니메이션 준비되면 요청.

---

## 3. `[ ]` 앉기(Crouch) 추가

- [ ] `IA_Crouch`(LCtrl, Hold) 입력을 IMC에 매핑 (참고: [progress_human.md](progress_human.md) §2)
- [ ] ABP_Mika에 Crouch 상태 추가 (Idle/Walk ↔ Crouch 블렌딩)
- [ ] 앉아서 사격 동작 확인
- [ ] 앉아서 턴(turn-in-place) 동작 확인
- [ ] C++ 로직 필요 여부 점검 (ACharacter 기본 Crouch 사용 가능한지 / 별도 상태 관리 필요한지)

> CLAUDE.md "진행중 > 3. 앉기 추가" 항목과 연동.
