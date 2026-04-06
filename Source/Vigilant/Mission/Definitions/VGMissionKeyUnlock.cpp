#include "VGMissionKeyUnlock.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"

AVGMissionKeyUnlock::AVGMissionKeyUnlock()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionKeyUnlock::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
	Super::OnGimmickStateChanged(Gimmick, Tag);
	if (Tag != VigilantMissionTags::GimmickCompleted) return;

	if (AreAllChestsUnlocked())
	{
		CompleteMission();
	}
}

bool AVGMissionKeyUnlock::AreAllChestsUnlocked() const
{
	for (AVGMissionGimmickBase* Gimmick : MissionGimmicks)
	{
		if (!Gimmick) continue;
		if (Gimmick->GetStateTag() != VigilantMissionTags::GimmickCompleted)
		{
			return false;
		}
	}
	return true;
}

