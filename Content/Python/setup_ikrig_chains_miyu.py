"""
IK Rig 체인 자동 세팅 스크립트
대상: Miyu Ver12/13 (Root_178, Hips_177 등 번호 붙은 본 구조)

사용법:
  UE 출력 로그 > Python 탭에서:
  exec(open(r'D:/UEProject/GameAnimationSample2/Content/Python/setup_ikrig_chains_miyu.py').read())
"""

import unreal

# ── 설정 ───────────────────────────────────────────────
IK_RIG_PATH = "/Game/HJ_Resources/IKs/IK_miyu_ver13"

RETARGET_ROOT = "Hips_177"

# (체인 이름, 시작 본, 끝 본)
CHAINS = [
    # Spine
    ("Spine",          "Spine_176",         "UpperChest_167"),
    ("Head",           "Neck_126",           "Head_125"),

    # Left Arm
    ("LeftClavicle",   "Shoulder_L_166",     "Shoulder_L_166"),
    ("LeftArm",        "UpperArm_L_165",     "Hand_L_163"),

    # Right Arm
    ("RightClavicle",  "Shoulder_R_146",     "Shoulder_R_146"),
    ("RightArm",       "UpperArm_R_145",     "Hand_R_142"),

    # Left Leg
    ("LeftLeg",        "UpperLeg_L_20",      "Foot_L_18"),

    # Right Leg
    ("RightLeg",       "UpperLeg_R_16",      "Foot_R_14"),

    # Left Fingers
    ("LeftThumb",      "Thumb_Proximal_L_153",  "Thumb_Distal_L_151"),
    ("LeftIndex",      "Index_Proximal_L_156",  "Index_Distal_L_154"),
    ("LeftMiddle",     "Middle_Proximal_L_159", "Middle_Distal_L_157"),
    ("LeftRing",       "Ring_Proximal_L_162",   "Ring_Distal_L_160"),
    ("LeftPinky",      "Little_Proximal_L_150", "Little_Distal_L_148"),

    # Right Fingers
    ("RightThumb",     "Thumb_Proximal_R_129",  "Thumb_Distal_R_127"),
    ("RightIndex",     "Index_Proximal_R_132",  "Index_Distal_R_130"),
    ("RightMiddle",    "Middle_Proximal_R_135", "Middle_Distal_R_133"),
    ("RightRing",      "Ring_Proximal_R_138",   "Ring_Distal_R_136"),
    ("RightPinky",     "Little_Proximal_R_141", "Little_Distal_R_139"),
]
# ───────────────────────────────────────────────────────


def setup_chains():
    ik_rig = unreal.load_asset(IK_RIG_PATH)
    if not ik_rig:
        unreal.log_error(f"IK Rig 못 찾음: {IK_RIG_PATH}")
        return

    controller = unreal.IKRigController.get_controller(ik_rig)

    with unreal.ScopedEditorTransaction("IK Rig 체인 자동 세팅 - Miyu") as t:

        # 기존 체인 전부 제거
        existing = controller.get_retarget_chains()
        for chain in existing:
            controller.remove_retarget_chain(chain.chain_name)
        unreal.log(f"기존 체인 {len(existing)}개 제거 완료")

        # 루트 설정
        controller.set_retarget_root(RETARGET_ROOT)
        unreal.log(f"Retarget Root: {RETARGET_ROOT}")

        # 체인 추가
        success = 0
        fail = 0
        for chain_name, start_bone, end_bone in CHAINS:
            result = controller.add_retarget_chain(
                unreal.Name(chain_name),
                unreal.Name(start_bone),
                unreal.Name(end_bone),
                unreal.Name("")
            )
            if result:
                unreal.log(f"  ✓ {chain_name} ({start_bone} → {end_bone})")
                success += 1
            else:
                unreal.log_warning(f"  ✗ {chain_name} 실패 — 본 이름 확인 필요")
                fail += 1

    unreal.log(f"\n완료! 성공: {success}개 / 실패: {fail}개")
    unreal.EditorAssetLibrary.save_asset(IK_RIG_PATH)
    unreal.log("저장 완료")


setup_chains()
