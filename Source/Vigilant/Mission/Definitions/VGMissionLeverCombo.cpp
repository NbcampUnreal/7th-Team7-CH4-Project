#include "VGMissionLeverCombo.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"

AVGMissionLeverCombo::AVGMissionLeverCombo()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionLeverCombo::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
	// 레버 상태 변경 시 CurrentOnLeverIndexes 갱신
	if (Gimmick->GetStateTag() == VigilantMissionTags::GimmickActive)
	{
		CurrentOnLeverIndexes.Add(Gimmick->GetGimmickIndex());
	}
	else
	{
		CurrentOnLeverIndexes.Remove(Gimmick->GetGimmickIndex());
	}
	
	
	if (CurrentOnLeverIndexes.Num() == RequiredOnLeverIndexes.Num()
		&& RequiredOnLeverIndexes.Includes(CurrentOnLeverIndexes))
	{
		CompleteMission();
	}
}

