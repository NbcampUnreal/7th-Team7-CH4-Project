#include "VGMissionGimmickBase.h"
#include "Net/UnrealNetwork.h"

AVGMissionGimmickBase::AVGMissionGimmickBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionGimmickBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, GimmickStateTag);
}

void AVGMissionGimmickBase::ReportConditionMet()
{
}

void AVGMissionGimmickBase::SetGimmickState(FGameplayTag NewStateTag)
{
	if (!HasAuthority())
	{
		return;
	}
	
	GimmickStateTag = NewStateTag;
	OnRep_GimmickStateTag();
}

void AVGMissionGimmickBase::OnRep_GimmickStateTag()
{
}

