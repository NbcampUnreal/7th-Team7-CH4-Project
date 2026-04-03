// Fill out your copyright notice in the Description page of Project Settings.


#include "VGGameplayTags.h"

//지금 아래건 예시 입니다!, 따옴표 안의 글자 오타 조심!
//캐릭터의 상태 및 태그
UE_DEFINE_GAMEPLAY_TAG(VigilantCharacter::Dodge, "Character.Action.Dodge");
UE_DEFINE_GAMEPLAY_TAG(VigilantCharacter::Invincible, "Character.State.Invincible");
UE_DEFINE_GAMEPLAY_TAG(VigilantCharacter::OccupiedLeftHand, "Character.State.Occupied.Left");
UE_DEFINE_GAMEPLAY_TAG(VigilantCharacter::OccupiedRightHand, "Character.State.Occupied.Right");


//UE_DEFINE_GAMEPLAY_TAG(VigilantTags::Sprint, "Character.Action.Movable.Sprint");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::MissionInactive, "Mission.State.Inactive");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::MissionActive, "Mission.State.Active");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::MissionCompleted, "Mission.State.Completed");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::PuzzleMission, "Mission.Type.Puzzle");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::CooperativeMission, "Mission.Type.Cooperative");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::CombatMission, "Mission.Type.Combat");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::GimmickInactive, "Mission.Gimmick.State.Inactive");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::GimmickActive, "Mission.Gimmick.State.Active");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::GimmickCompleted, "Mission.Gimmick.State.Completed");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::PressureGimmick, "Mission.Gimmick.Type.Pressure");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::StatueGimmick, "Mission.Gimmick.Type.Statue");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::LeverGimmick, "Mission.Gimmick.Type.Lever");

// 장비 태그 정의
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::Weapon_OneHand, "Item.Type.Weapon.OneHand");
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::Weapon_TwoHand, "Item.Type.Weapon.TwoHand");
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::Weapon_Shield, "Item.Type.Weapon.Shield");
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::Item_Mission, "Item.Type.Special.Mission");

VGGameplayTags::VGGameplayTags()
{
}

VGGameplayTags::~VGGameplayTags()
{
}
