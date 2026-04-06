#include "VGMissionAllGimmicksClear.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"
#include "Common/VGGameplayTags.h"

AVGMissionAllGimmicksClear::AVGMissionAllGimmicksClear()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionAllGimmicksClear::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
	Super::OnGimmickStateChanged(Gimmick, Tag);
	if (Tag == VigilantMissionTags::GimmickCompleted)
	{
		if (AreAllGimmickCompleted())
		{
			CompleteMission();
		}
	}
}

bool AVGMissionAllGimmicksClear::AreAllGimmickCompleted() const
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



