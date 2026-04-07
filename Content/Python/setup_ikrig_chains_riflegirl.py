"""
IK Rig 체인 자동 세팅 스크립트
대상: RifleGirl Ver05 (UE5 표준 본 구조)

사용법:
  UE 출력 로그 > Python 탭에서:
  exec(open(r'D:/UEProject/GameAnimationSample2/Content/Python/setup_ikrig_chains_riflegirl.py').read())
"""

import unreal

IK_RIG_PATH = "/Game/HJ_Resources/IKs/IK_RifleGirl"

RETARGET_ROOT = "root"  # UE5 표준 Mannequin은 root → pelvis 구조. 없으면 "pelvis"로 교체

CHAINS = [
    # Root / Pelvis
    ("Root",          "pelvis",      "pelvis",      ""),
    ("Pelvis",        "pelvis",      "pelvis",      ""),

    # Spine / Neck / Head
    ("Spine",         "spine_01",    "spine_03",    ""),
    ("Neck",          "neck_01",     "neck_01",     ""),
    ("Head",          "head",        "head",        ""),

    # Left Arm
    ("LeftClavicle",  "clavicle_l",  "clavicle_l",  ""),
    ("LeftArm",       "upperarm_l",  "hand_l",      "LeftHandIK"),

    # Right Arm
    ("RightClavicle", "clavicle_r",  "clavicle_r",  ""),
    ("RightArm",      "upperarm_r",  "hand_r",      "RightHandIK"),

    # Left Leg
    ("LeftLeg",       "thigh_l",     "foot_l",      "LeftFootIK"),
    ("LeftToe",       "ball_l",      "ball_l",      ""),

    # Right Leg
    ("RightLeg",      "thigh_r",     "foot_r",      "RightFootIK"),
    ("RightToe",      "ball_r",      "ball_r",      ""),

    # Left Fingers
    ("LeftThumb",     "thumb_01_l",  "thumb_03_l",  ""),
    ("LeftIndex",     "index_01_l",  "index_03_l",  ""),
    ("LeftMiddle",    "middle_01_l", "middle_03_l", ""),
    ("LeftRing",      "ring_01_l",   "ring_03_l",   ""),
    ("LeftPinky",     "pinky_01_l",  "pinky_03_l",  ""),

    # Right Fingers
    ("RightThumb",    "thumb_01_r",  "thumb_03_r",  ""),
    ("RightIndex",    "index_01_r",  "index_03_r",  ""),
    ("RightMiddle",   "middle_01_r", "middle_03_r", ""),
    ("RightRing",     "ring_01_r",   "ring_03_r",   ""),
    ("RightPinky",    "pinky_01_r",  "pinky_03_r",  ""),
]

IK_GOALS = [
    ("LeftHandIK",  "hand_l"),
    ("RightHandIK", "hand_r"),
    ("LeftFootIK",  "foot_l"),
    ("RightFootIK", "foot_r"),
]


def setup_chains():
    ik_rig = unreal.load_asset(IK_RIG_PATH)
    if not ik_rig:
        unreal.log_error(f"IK Rig 못 찾음: {IK_RIG_PATH}")
        return

    controller = unreal.IKRigController.get_controller(ik_rig)

    with unreal.ScopedEditorTransaction("IK Rig 체인 자동 세팅 - RifleGirl") as _:

        existing = controller.get_retarget_chains()
        for chain in existing:
            controller.remove_retarget_chain(chain.chain_name)
        unreal.log(f"기존 체인 {len(existing)}개 제거")

        controller.set_retarget_root(RETARGET_ROOT)
        unreal.log(f"Retarget Root: {RETARGET_ROOT}")

        success = 0
        fail = 0
        for chain_name, start_bone, end_bone, ik_goal in CHAINS:
            result = controller.add_retarget_chain(
                unreal.Name(chain_name),
                unreal.Name(start_bone),
                unreal.Name(end_bone),
                unreal.Name(ik_goal)
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
