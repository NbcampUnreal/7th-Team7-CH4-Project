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

namespace VigilantCharacter
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(RotateLock);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(LockOn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Guard);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PerfectGuard);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Invincible);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Falling);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stunned);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sprint);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(OccupiedLeftHand);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(OccupiedRightHand);
}

namespace VigilantBoss
{
	// 보스 스킬 상태
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Casting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SkillCooldown_Q);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SkillCooldown_E);
	
	// 보스 사망 상태
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dead);
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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GimmickInactive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GimmickActive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GimmickCompleted);
	// 미션 기믹 타입
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PressureGimmick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(StatueGimmick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(LeverGimmick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ChestGimmick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AltarGimmick);
	// 미션 아이템 상태
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemInactive);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemCarried);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemPlaced); 
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemUsed); 
	// 미션 아이템 타입
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(KeyItem);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CarryItem1);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CarryItem2);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CarryItem3);
	
}

// 게임 페이즈 관리용
namespace VigilantPhaseTags 
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PhaseLobby);
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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DuelParticipant);  // 막고라 당사자
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DuelSpectator);    // 막고라 관람객
	
	// 막고라 패널티 (후에 필요하면 주석 제거)
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(PenaltyChatBan);        // 채팅 금지
}

namespace VigilantEquipmentTags
{
	// 장비 태그 선언
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_OneHand);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_TwoHand);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Shield);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Mission);
}

class VIGILANT_API VGGameplayTags
{
public:
	VGGameplayTags();
	~VGGameplayTags();
};
