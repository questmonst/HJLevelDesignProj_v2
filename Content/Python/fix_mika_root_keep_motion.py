"""
미카 리타겟 애니의 루트 본(survivor_teenangst_arm) 크기 1 → 100 보정 (루트 이동량 유지판).

원인: IK Retargeter 내보내기는 크기를 뺀 포즈로 계산해 루트 크기를 1로 굽는다
      (미카 스켈레톤 루트 크기는 100) → 게임에서 1/100로 쪼그라든다.
기존 fix_mika_root_scale.py는 루트 트랙을 지워서 고치지만, 출발·정지처럼 루트 이동량이 있는
애니는 이동량까지 사라진다. 이 스크립트는 트랙을 남기고 크기 100, 이동량 ×100으로 다시 쓴다.
(내보내기 결과의 루트 이동량은 크기 1 기준이라 100배 해야 몸과 같은 단위가 된다)

이미 크기가 100인 애니는 건너뛴다 (두 번 돌려도 안전).
"""

import unreal

# ── 설정 ──────────────────────────────────────────────────────────────────────
ANIM_FOLDER = "/Game/V2_HJContents/V2Resource/Character/mika/RifleActionPack/Anims"
ROOT_BONE   = "survivor_teenangst_arm"
# ─────────────────────────────────────────────────────────────────────────────

AL = unreal.AnimationLibrary
fixed = skipped = 0

for asset_data in unreal.AssetRegistryHelpers.get_asset_registry().get_assets_by_path(ANIM_FOLDER, recursive=True):
    if str(asset_data.asset_class_path.asset_name) != "AnimSequence":
        continue
    anim = asset_data.get_asset()

    if ROOT_BONE not in [str(n) for n in AL.get_animation_track_names(anim)]:
        skipped += 1
        continue
    if abs(AL.get_bone_pose_for_frame(anim, ROOT_BONE, 0, False).scale3d.x - 100.0) < 0.01:
        skipped += 1
        continue

    pos, rot, scl = [], [], []
    for frame in range(AL.get_num_keys(anim)):
        t = AL.get_bone_pose_for_frame(anim, ROOT_BONE, frame, False)
        pos.append(unreal.Vector(t.translation.x * 100.0, t.translation.y * 100.0, t.translation.z * 100.0))
        rot.append(t.rotation)
        scl.append(unreal.Vector(100.0, 100.0, 100.0))
    anim.controller.set_bone_track_keys(ROOT_BONE, pos, rot, scl, False)
    unreal.EditorAssetLibrary.save_loaded_asset(anim, False)
    fixed += 1

print(f"완료 — 수정: {fixed}개 / 건너뜀: {skipped}개")
