// Fill out your copyright notice in the Description page of Project Settings.


#include "VGGameplayTags.h"

//지금 아래건 예시 입니다!, 따옴표 안의 글자 오타 조심!
//UE_DEFINE_GAMEPLAY_TAG(VigilantTags::Sprint, "Character.Action.Movable.Sprint");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::MissionInactive, "Mission.State.Inactive");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::MissionActive, "Mission.State.Active");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::MissionCompleted, "Mission.State.Completed");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::PuzzleMission, "Mission.Type.Puzzle");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::CooperativeMission, "Mission.Type.Cooperative");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::CombatMission, "Mission.Type.Combat");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::GimmickInative, "Mission.Gimmick.State.Inactive");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::GimmickActive, "Mission.Gimmick.State.Active");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::GimmickCompleted, "Mission.Gimmick.State.Completed");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::PressureGimmick, "Mission.Gimmick.Type.Pressure");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::StatueGimmick, "Mission.Gimmick.Type.Statue");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::LeverGimmick, "Mission.Gimmick.Type.Lever");

// 장비 상태 태그 정의
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::State_Equipped_Weapon, "State.Equipped.Weapon");
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::State_Equipped_Shield, "State.Equipped.Shield");
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::State_Equipped_TwoHanded, "State.Equipped.TwoHanded");
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::State_Equipped_MissionItem, "State.Equipped.MissionItem");

VGGameplayTags::VGGameplayTags()
{
}

VGGameplayTags::~VGGameplayTags()
{
}
