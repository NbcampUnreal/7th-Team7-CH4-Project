// Fill out your copyright notice in the Description page of Project Settings.


#include "VGGameplayTags.h"

//지금 아래건 예시 입니다!, 따옴표 안의 글자 오타 조심!
//캐릭터의 상태 및 태그
UE_DEFINE_GAMEPLAY_TAG(VigilantCharacter::Dodge, "Character.Action.Dodge");
UE_DEFINE_GAMEPLAY_TAG(VigilantCharacter::Invincible, "Character.State.Invincible");
UE_DEFINE_GAMEPLAY_TAG(VigilantCharacter::Falling, "Character.State.Falling");
UE_DEFINE_GAMEPLAY_TAG(VigilantCharacter::OccupiedLeftHand, "Character.State.Occupied.Left");
UE_DEFINE_GAMEPLAY_TAG(VigilantCharacter::OccupiedRightHand, "Character.State.Occupied.Right");

// 보스 태그 정의
UE_DEFINE_GAMEPLAY_TAG(VigilantBoss::Casting, "Boss.State.Casting");
UE_DEFINE_GAMEPLAY_TAG(VigilantBoss::SkillCooldown_Q, "Boss.Skill.Cooldown.Q");
UE_DEFINE_GAMEPLAY_TAG(VigilantBoss::SkillCooldown_E, "Boss.Skill.Cooldown.E");

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
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::ChestGimmick, "Mission.Gimmick.Type.Chest");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::AltarGimmick, "Mission.Gimmick.Type.Altar");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::ItemInactive, "Mission.Item.State.Inactive");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::ItemCarried, "Mission.Item.State.Carried");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::ItemPlaced, "Mission.Item.State.Placed");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::ItemUsed, "Mission.Item.State.Used");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::KeyItem, "Mission.Item.Type.Key");
UE_DEFINE_GAMEPLAY_TAG(VigilantMissionTags::CarryItem, "Mission.Item.Type.Carry");

// 장비 태그 정의
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::Weapon_OneHand, "Item.Type.Weapon.OneHand");
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::Weapon_TwoHand, "Item.Type.Weapon.TwoHand");
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::Weapon_Shield, "Item.Type.Weapon.Shield");
UE_DEFINE_GAMEPLAY_TAG(VigilantEquipmentTags::Item_Mission, "Item.Type.Special.Mission");

// 게임 페이즈 관련
UE_DEFINE_GAMEPLAY_TAG(VigilantPhaseTags::PhaseLobby, "Phase.State.Lobby");
UE_DEFINE_GAMEPLAY_TAG(VigilantPhaseTags::PhaseMission, "Phase.State.Mission");
UE_DEFINE_GAMEPLAY_TAG(VigilantPhaseTags::PhaseDuel,    "Phase.State.Duel");
UE_DEFINE_GAMEPLAY_TAG(VigilantPhaseTags::PhaseVote,    "Phase.State.Vote");
UE_DEFINE_GAMEPLAY_TAG(VigilantPhaseTags::PhaseCombat,  "Phase.State.Combat");

// 플레이어 역할 및 상태
UE_DEFINE_GAMEPLAY_TAG(VigilantRoleTags::Citizen,	  "Role.Player.Citizen");
UE_DEFINE_GAMEPLAY_TAG(VigilantRoleTags::Mafia,		  "Role.Player.Mafia");
UE_DEFINE_GAMEPLAY_TAG(VigilantRoleTags::Alive,		  "Role.Status.Alive");
UE_DEFINE_GAMEPLAY_TAG(VigilantRoleTags::Dead,		  "Role.Status.Dead");

// 플레이어 특수 상태
UE_DEFINE_GAMEPLAY_TAG(VigilantStateTags::DuelParticipant, "State.Condition.Duel.Participant");
UE_DEFINE_GAMEPLAY_TAG(VigilantStateTags::DuelSpectator,   "State.Condition.Duel.Spectator");

// 막고라 패널티 (후에 사용하면 주석 제거)
// UE_DEFINE_GAMEPLAY_TAG(VigilantStateTags::PenaltyChatBan, "State.Penalty.ChatBan");

VGGameplayTags::VGGameplayTags()
{
}

VGGameplayTags::~VGGameplayTags()
{
}
