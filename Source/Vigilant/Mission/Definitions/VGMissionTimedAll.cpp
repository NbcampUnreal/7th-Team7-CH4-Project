#include "VGMissionTimedAll.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"

AVGMissionTimedAll::AVGMissionTimedAll()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionTimedAll::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
	if (Tag != VigilantMissionTags::GimmickActive)
	{
		return;
	}
	
	if (CurrentStateTag == VigilantMissionTags::MissionInactive)
	{
		StartTimer();
		SetMissionState(VigilantMissionTags::MissionActive);
	}
	
	if (AreAllGimmickActive())
	{
		CompleteMission();
	}
}

void AVGMissionTimedAll::StartTimer()
{
	GetWorldTimerManager().SetTimer(
		MissionTimerHandle,
		this,
		&AVGMissionTimedAll::OnTimerExpired,
		TimeLimit,  // MissionBase에 이미 있는 변수
		false);
}

void AVGMissionTimedAll::OnTimerExpired()
{
	SetMissionState(VigilantMissionTags::MissionInactive);
	for (AVGMissionGimmickBase* Gimmick : MissionGimmicks)
	{
		if (Gimmick)
		{
			Gimmick->ResetGimmickState();
		}
	}
}

bool AVGMissionTimedAll::AreAllGimmickActive() const
{
	for (AVGMissionGimmickBase* Gimmick : MissionGimmicks)
	{
		if (!Gimmick) continue;
		if (Gimmick->GetStateTag() != VigilantMissionTags::GimmickActive &&
			Gimmick->GetStateTag() != VigilantMissionTags::GimmickCompleted)
		{
			return false;
		}
	}
	return true;
}

