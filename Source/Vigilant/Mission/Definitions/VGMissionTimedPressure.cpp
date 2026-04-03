#include "VGMissionTimedPressure.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"

AVGMissionTimedPressure::AVGMissionTimedPressure()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionTimedPressure::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
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
	
	if (AreAllPressuresActive())
	{
		CompleteMission();
	}
}

void AVGMissionTimedPressure::StartTimer()
{
	GetWorldTimerManager().SetTimer(
		MissionTimerHandle,
		this,
		&AVGMissionTimedPressure::OnTimerExpired,
		TimeLimit,  // MissionBase에 이미 있는 변수
		false);
}

void AVGMissionTimedPressure::OnTimerExpired()
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

bool AVGMissionTimedPressure::AreAllPressuresActive() const
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

