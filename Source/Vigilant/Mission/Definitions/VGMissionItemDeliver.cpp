#include "VGMissionItemDeliver.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"

AVGMissionItemDeliver::AVGMissionItemDeliver()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionItemDeliver::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
	Super::OnGimmickStateChanged(Gimmick, Tag);
	
	if (Tag != VigilantMissionTags::GimmickCompleted)
	{
		return;
	}
	
	if (AreAllAltarsPlaced())
	{
		CompleteMission();
	}
}

bool AVGMissionItemDeliver::AreAllAltarsPlaced() const
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


