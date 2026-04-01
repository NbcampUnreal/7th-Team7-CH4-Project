// Fill out your copyright notice in the Description page of Project Settings.


#include "VGGameplayTags.h"

//지금 아래건 예시 입니다!, 따옴표 안의 글자 오타 조심!
//UE_DEFINE_GAMEPLAY_TAG(VigilantTags::Sprint, "Character.Action.Movable.Sprint");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::MissionInactive, "Mission.State.Inactive");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::MissionActive, "Mission.State.Active");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::MissionCompleted, "Mission.State.Completed");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::PuzzleMission, "Mission.Type.Puzzle");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::CooperativeMission, "Mission.Type.Cooperative");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::CombatMission, "Mission.Type.Combat");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::GimmickInative, "Mission.Gimmick.State.Inactive");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::GimmickAtive, "Mission.Gimmick.State.Active");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::GimmickCompleted, "Mission.Gimmick.State.Completed");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::PressureGimmick, "Mission.Gimmick.Type.Pressure");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::StatueGimmick, "Mission.Gimmick.Type.Statue");
UE_DEFINE_GAMEPLAY_TAG(VigilantTags::LeverGimmick, "Mission.Gimmick.Type.Lever");


VGGameplayTags::VGGameplayTags()
{
}

VGGameplayTags::~VGGameplayTags()
{
}
