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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GimmickInactive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GimmickActive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GimmickCompleted);
	// 미션 기믹 타입
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PressureGimmick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(StatueGimmick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(LeverGimmick);
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
