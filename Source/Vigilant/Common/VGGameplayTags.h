// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
/**
 * 
/*
 필요한 태그를 미리 선언	
 사용법 네임스페이스::설정한이름 VigilantTags::Sprint
 
 정의방법 h에서 캐싱할 이름, cpp에서 태그 등록
*/
namespace VigilantTags
{
	//예시
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sprint);
	
	
}

namespace VigilantMissionTags
{
	// 미션 상태
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(MissionInactive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(MissionActive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(MissionCompleted);
	// 미션 타입
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PuzzleMission);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CooperativeMission);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CombatMission);
	// 미션 기믹 상태
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GimmickInative);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GimmickActive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GimmickCompleted);
	// 미션 기믹 타입
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PressureGimmick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(StatueGimmick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(LeverGimmick);
}

// 게임 페이즈 관리용
namespace VigilantPhaseTags 
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PhaseMission);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PhaseDuel);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PhaseVote);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PhaseCombat);
}

// 플레이어 역할 및 죽음 관리용
namespace VigilantRoleTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Citizen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mafia);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Alive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dead);
}

// 플레이어 상태 및 행동 제어용
namespace VigilantStateTags
{
	// 행동 제한
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DisableAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DisableInteract);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DisableMove);
	
	// 특수 상태
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Invincible);          // 무적
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DuelParticipant);  // 막고라 당사자
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DuelSpectator);    // 막고라 관람객
	
	// 막고라 패널티 (후에 필요하면 주석 제거)
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(PenaltyChatBan);        // 채팅 금지
}

class VIGILANT_API VGGameplayTags
{
public:
	VGGameplayTags();
	~VGGameplayTags();
};
