#include "VGMissionStatueAlign.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"

AVGMissionStatueAlign::AVGMissionStatueAlign()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionStatueAlign::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
	Super::OnGimmickStateChanged(Gimmick, Tag);
	if (Tag == VigilantMissionTags::GimmickCompleted)
	{
		if (AreAllStatuesAligned())
		{
			CompleteMission();
		}
	}
}

bool AVGMissionStatueAlign::AreAllStatuesAligned() const
{
	for (AVGMissionGimmickBase* Gimmick : MissionGimmicks)
	{
		if (!Gimmick)
		{
			continue;
		}
		
		if (Gimmick->GetStateTag() != VigilantMissionTags::GimmickCompleted)
		{
			return false;
		}
	}
	return true;
}

