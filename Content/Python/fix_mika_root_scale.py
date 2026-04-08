"""
익스포트된 미카 리타겟 애니메이션의 루트 본(survivor_teenangst_ARM) 스케일을
100으로 강제 수정하는 스크립트.

원인: IK Retargeter가 RifleGirl(스케일 1.0) 기준으로 루트 본 스케일을 구워서
      게임 내에서 캐릭터가 1/100 크기로 축소되는 문제.
"""

import unreal

# ── 설정 ──────────────────────────────────────────────────────────────────────
ANIM_FOLDER   = "/Game/V2_HJContents/V2Resource/Character/mika/ThuggedAnims"
ROOT_BONE     = "survivor_teenangst_arm"
TARGET_SCALE  = unreal.Vector(100.0, 100.0, 100.0)
# ─────────────────────────────────────────────────────────────────────────────

anim_lib   = unreal.AnimationLibrary
asset_reg  = unreal.AssetRegistryHelpers.get_asset_registry()
asset_lib  = unreal.EditorAssetLibrary

assets = asset_reg.get_assets_by_path(ANIM_FOLDER, recursive=True)

fixed  = 0
skipped = 0

for asset_data in assets:
    if str(asset_data.asset_class_path.asset_name) != "AnimSequence":
        continue

    anim = asset_data.get_asset()

    track_names = [str(n) for n in anim_lib.get_animation_track_names(anim)]
    if ROOT_BONE not in track_names:
        print(f"[SKIP] {anim.get_name()} — 루트 본 트랙 없음")
        skipped += 1
        continue

    num_frames = anim_lib.get_num_keys(anim)

    poses = []
    times = []

    # 루트 본 트랙 제거 → 스켈레톤 레퍼런스 포즈(스케일 100) 사용
    anim_lib.remove_bone_animation(anim, ROOT_BONE, False)
    asset_lib.save_asset(anim.get_path_name())
    fixed += 1
    print(f"[FIX] {anim.get_name()}")

print(f"\n완료 — 수정: {fixed}개 / 스킵: {skipped}개")
