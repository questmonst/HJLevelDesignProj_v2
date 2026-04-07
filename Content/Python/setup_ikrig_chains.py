"""
IK Rig 체인 자동 세팅 스크립트
대상: Misono_Mika (ValveBiped_Bip01_* 본 구조)

사용법:
  UE 출력 로그 > Python 탭에서:
  exec(open(r'D:/UEProject/GameAnimationSample2/Content/Python/setup_ikrig_chains.py').read())
"""

import unreal
import time

# ── 설정 ───────────────────────────────────────────────
IK_RIG_PATH = "/Game/V2_HJContents/V2Resource/Character/mika/Ver09/IK_Misono_Mika1_test"

RETARGET_ROOT = "survivor_teenangst_ARM"

# (체인 이름, 시작 본, 끝 본)
# 체인 이름은 UEFN Mannequin IK Rig 기준으로 맞춤
CHAINS = [
    # Root / Pelvis (실제 루트 본 + 골반)
    ("Root",           "survivor_teenangst_ARM",        "survivor_teenangst_ARM",      ""),
    ("Pelvis",         "ValveBiped_Bip01_Pelvis",       "ValveBiped_Bip01_Pelvis",     ""),

    # Spine / Neck / Head
    ("Spine",          "ValveBiped_Bip01_Spine",       "ValveBiped_Bip01_Spine4",     ""),
    ("Neck",           "ValveBiped_Bip01_Neck1",        "ValveBiped_Bip01_Neck1",      ""),
    ("Head",           "ValveBiped_Bip01_Head1",        "ValveBiped_Bip01_Head1",      ""),

    # Left Arm
    ("LeftClavicle",   "ValveBiped_Bip01_L_Clavicle",  "ValveBiped_Bip01_L_Clavicle", ""),
    ("LeftArm",        "ValveBiped_Bip01_L_UpperArm",  "ValveBiped_Bip01_L_Hand",     "LeftHandIK"),

    # Right Arm
    ("RightClavicle",  "ValveBiped_Bip01_R_Clavicle",  "ValveBiped_Bip01_R_Clavicle", ""),
    ("RightArm",       "ValveBiped_Bip01_R_UpperArm",  "ValveBiped_Bip01_R_Hand",     "RightHandIK"),

    # Left Leg
    ("LeftLeg",        "ValveBiped_Bip01_L_Thigh",     "ValveBiped_Bip01_L_Foot",    "LeftFootIK"),
    ("LeftToe",        "ValveBiped_Bip01_L_Toe0",       "ValveBiped_Bip01_L_Toe0",    ""),

    # Right Leg
    ("RightLeg",       "ValveBiped_Bip01_R_Thigh",     "ValveBiped_Bip01_R_Foot",    "RightFootIK"),
    ("RightToe",       "ValveBiped_Bip01_R_Toe0",       "ValveBiped_Bip01_R_Toe0",    ""),

    # Left Fingers
    # ValveBiped: Finger0=Thumb, Finger1=Index, Finger2=Middle, Finger3=Ring, Finger4=Pinky
    ("LeftThumb",      "ValveBiped_Bip01_L_Finger0",   "ValveBiped_Bip01_L_Finger02", ""),
    ("LeftIndex",      "ValveBiped_Bip01_L_Finger1",   "ValveBiped_Bip01_L_Finger12", ""),
    ("LeftMiddle",     "ValveBiped_Bip01_L_Finger2",   "ValveBiped_Bip01_L_Finger22", ""),
    ("LeftRing",       "ValveBiped_Bip01_L_Finger3",   "ValveBiped_Bip01_L_Finger32", ""),
    ("LeftPinky",      "ValveBiped_Bip01_L_Finger4",   "ValveBiped_Bip01_L_Finger42", ""),

    # Right Fingers
    ("RightThumb",     "ValveBiped_Bip01_R_Finger0",   "ValveBiped_Bip01_R_Finger02", ""),
    ("RightIndex",     "ValveBiped_Bip01_R_Finger1",   "ValveBiped_Bip01_R_Finger12", ""),
    ("RightMiddle",    "ValveBiped_Bip01_R_Finger2",   "ValveBiped_Bip01_R_Finger22", ""),
    ("RightRing",      "ValveBiped_Bip01_R_Finger3",   "ValveBiped_Bip01_R_Finger32", ""),
    ("RightPinky",     "ValveBiped_Bip01_R_Finger4",   "ValveBiped_Bip01_R_Finger42", ""),
]
# ── FBIK 솔버 설정 ─────────────────────────────────────
# (본 이름, 설정 타입)
# 설정 타입: "root" / "excluded" / None(기본)
FBIK_BONES = [
    ("ValveBiped_Bip01_Pelvis",    "root"),
    ("ValveBiped_Bip01_Spine",     None),
    ("ValveBiped_Bip01_Spine1",    None),
    ("ValveBiped_Bip01_Spine2",    None),
    ("ValveBiped_Bip01_Spine4",    None),
    ("ValveBiped_Bip01_Neck1",     None),
    ("ValveBiped_Bip01_Head1",     None),
    ("ValveBiped_Bip01_L_Clavicle",None),
    ("ValveBiped_Bip01_L_UpperArm",None),
    ("ValveBiped_Bip01_L_Forearm", None),
    ("ValveBiped_Bip01_L_Hand",    None),
    ("ValveBiped_Bip01_R_Clavicle",None),
    ("ValveBiped_Bip01_R_UpperArm",None),
    ("ValveBiped_Bip01_R_Forearm", None),
    ("ValveBiped_Bip01_R_Hand",    None),
    ("ValveBiped_Bip01_L_Thigh",   None),
    ("ValveBiped_Bip01_L_Calf",    None),
    ("ValveBiped_Bip01_L_Foot",    None),
    ("ValveBiped_Bip01_R_Thigh",   None),
    ("ValveBiped_Bip01_R_Calf",    None),
    ("ValveBiped_Bip01_R_Foot",    None),
]

# IK 목표: (goal 이름, 연결할 본 이름)
IK_GOALS = [
    ("LeftHandIK",  "ValveBiped_Bip01_L_Hand"),
    ("RightHandIK", "ValveBiped_Bip01_R_Hand"),
    ("LeftFootIK",  "ValveBiped_Bip01_L_Foot"),
    ("RightFootIK", "ValveBiped_Bip01_R_Foot"),
]
# ───────────────────────────────────────────────────────


def setup_chains():
    start_time = time.time()
    ik_rig = unreal.load_asset(IK_RIG_PATH)
    if not ik_rig:
        unreal.log_error(f"IK Rig 못 찾음: {IK_RIG_PATH}")
        return

    controller = unreal.IKRigController.get_controller(ik_rig)

    with unreal.ScopedEditorTransaction("IK Rig 체인 자동 세팅") as t:

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
        for chain_name, start_bone, end_bone, ik_goal in CHAINS:
            result = controller.add_retarget_chain(
                unreal.Name(chain_name),
                unreal.Name(start_bone),
                unreal.Name(end_bone),
                unreal.Name(ik_goal)
            )
            if result:
                goal_str = f" [IK: {ik_goal}]" if ik_goal else ""
                unreal.log(f"  ✓ {chain_name} ({start_bone} → {end_bone}){goal_str}")
                success += 1
            else:
                unreal.log_warning(f"  ✗ {chain_name} 실패 — 본 이름 확인 필요")
                fail += 1

    # ── FBIK 솔버 추가 ──────────────────────────────────
    unreal.log("\n[FBIK 솔버 세팅 시작]")

    # 기존 솔버 제거
    solver_count = controller.get_num_solvers()
    for i in range(solver_count - 1, -1, -1):
        controller.remove_solver(i)
    unreal.log(f"기존 솔버 {solver_count}개 제거")

    with unreal.ScopedEditorTransaction("FBIK 솔버 세팅") as _:

        # Full Body IK 솔버 추가 (클래스 직접 전달)
        solver_index = controller.add_solver(unreal.IKRigFullBodyIKSolver)
        unreal.log(f"FBIK 솔버 추가됨 (index: {solver_index})")

        # Root 본 설정
        root_bone = FBIK_BONES[0][0]
        controller.set_root_bone(unreal.Name(root_bone), solver_index)
        unreal.log(f"Root 본: {root_bone}")

        # IK Goal 추가 및 솔버 연결
        # FBIK는 전체 뼈대를 자동 포함하므로 본을 일일이 추가할 필요 없음
        for goal_name, bone_name in IK_GOALS:
            controller.add_new_goal(unreal.Name(goal_name), unreal.Name(bone_name))
            controller.connect_goal_to_solver(unreal.Name(goal_name), solver_index)
            unreal.log(f"  IK Goal: {goal_name} → {bone_name}")

        unreal.log(f"IK Goal {len(IK_GOALS)}개 생성 완료")

    elapsed = time.time() - start_time
    unreal.log(f"\n전체 완료! 성공: {success}개 / 실패: {fail}개 | 소요 시간: {elapsed:.2f}초")
    unreal.EditorAssetLibrary.save_asset(IK_RIG_PATH)
    unreal.log("저장 완료")


setup_chains()
