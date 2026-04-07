"""
IK Rig 체인 자동 세팅 스크립트
대상: IK_Misono_Mika1_test2 (ValveBiped_Bip01_* 본 구조)
용도: RifleGirl → Mika 리타겟의 타겟 IK Rig

사용법:
  exec(open(r'D:/UEProject/GameAnimationSample2/Content/Python/setup_ikrig_chains_mika_test2.py').read())
"""

import unreal
import time

IK_RIG_PATH = "/Game/V2_HJContents/V2Resource/Character/mika/Ver09/IK_Misono_Mika1_test2"

RETARGET_ROOT = "survivor_teenangst_ARM"

CHAINS = [
    ("Root",           "survivor_teenangst_ARM",        "survivor_teenangst_ARM",      ""),
    ("Pelvis",         "ValveBiped_Bip01_Pelvis",       "ValveBiped_Bip01_Pelvis",     ""),
    ("Spine",          "ValveBiped_Bip01_Spine",        "ValveBiped_Bip01_Spine4",     ""),
    ("Neck",           "ValveBiped_Bip01_Neck1",        "ValveBiped_Bip01_Neck1",      ""),
    ("Head",           "ValveBiped_Bip01_Head1",        "ValveBiped_Bip01_Head1",      ""),
    ("LeftClavicle",   "ValveBiped_Bip01_L_Clavicle",  "ValveBiped_Bip01_L_Clavicle", ""),
    ("LeftArm",        "ValveBiped_Bip01_L_UpperArm",  "ValveBiped_Bip01_L_Hand",     "LeftHandIK"),
    ("RightClavicle",  "ValveBiped_Bip01_R_Clavicle",  "ValveBiped_Bip01_R_Clavicle", ""),
    ("RightArm",       "ValveBiped_Bip01_R_UpperArm",  "ValveBiped_Bip01_R_Hand",     "RightHandIK"),
    ("LeftLeg",        "ValveBiped_Bip01_L_Thigh",     "ValveBiped_Bip01_L_Foot",     "LeftFootIK"),
    ("LeftToe",        "ValveBiped_Bip01_L_Toe0",      "ValveBiped_Bip01_L_Toe0",     ""),
    ("RightLeg",       "ValveBiped_Bip01_R_Thigh",     "ValveBiped_Bip01_R_Foot",     "RightFootIK"),
    ("RightToe",       "ValveBiped_Bip01_R_Toe0",      "ValveBiped_Bip01_R_Toe0",     ""),
    ("LeftThumb",      "ValveBiped_Bip01_L_Finger0",   "ValveBiped_Bip01_L_Finger02", ""),
    ("LeftIndex",      "ValveBiped_Bip01_L_Finger1",   "ValveBiped_Bip01_L_Finger12", ""),
    ("LeftMiddle",     "ValveBiped_Bip01_L_Finger2",   "ValveBiped_Bip01_L_Finger22", ""),
    ("LeftRing",       "ValveBiped_Bip01_L_Finger3",   "ValveBiped_Bip01_L_Finger32", ""),
    ("LeftPinky",      "ValveBiped_Bip01_L_Finger4",   "ValveBiped_Bip01_L_Finger42", ""),
    ("RightThumb",     "ValveBiped_Bip01_R_Finger0",   "ValveBiped_Bip01_R_Finger02", ""),
    ("RightIndex",     "ValveBiped_Bip01_R_Finger1",   "ValveBiped_Bip01_R_Finger12", ""),
    ("RightMiddle",    "ValveBiped_Bip01_R_Finger2",   "ValveBiped_Bip01_R_Finger22", ""),
    ("RightRing",      "ValveBiped_Bip01_R_Finger3",   "ValveBiped_Bip01_R_Finger32", ""),
    ("RightPinky",     "ValveBiped_Bip01_R_Finger4",   "ValveBiped_Bip01_R_Finger42", ""),
]

IK_GOALS = [
    ("LeftHandIK",  "ValveBiped_Bip01_L_Hand"),
    ("RightHandIK", "ValveBiped_Bip01_R_Hand"),
    ("LeftFootIK",  "ValveBiped_Bip01_L_Foot"),
    ("RightFootIK", "ValveBiped_Bip01_R_Foot"),
]


def setup_chains():
    start_time = time.time()
    ik_rig = unreal.load_asset(IK_RIG_PATH)
    if not ik_rig:
        unreal.log_error(f"IK Rig 못 찾음: {IK_RIG_PATH}")
        return

    controller = unreal.IKRigController.get_controller(ik_rig)

    with unreal.ScopedEditorTransaction("IK Rig 체인 세팅 - Mika test2") as _:
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
                unreal.log(f"  ✓ {chain_name}")
                success += 1
            else:
                unreal.log_warning(f"  ✗ {chain_name} 실패")
                fail += 1

    elapsed = time.time() - start_time
    unreal.log(f"\n완료! 성공: {success} / 실패: {fail} | {elapsed:.2f}초")
    unreal.EditorAssetLibrary.save_asset(IK_RIG_PATH)
    unreal.log("저장 완료")


setup_chains()
