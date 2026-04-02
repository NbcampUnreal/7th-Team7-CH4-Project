#include "VGMissionGimmickLever.h"
#include "Common/VGGameplayTags.h"

AVGMissionGimmickLever::AVGMissionGimmickLever()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AVGMissionGimmickLever::IsActivated() const
{
	return (GimmickStateTag == VigilantMissionTags::GimmickActive);
}

bool AVGMissionGimmickLever::CanInteractWith(AVGCharacterBase* Interactor) const
{
	if (bIsOneWay && IsActivated())
	{
		return false;
	}
	
	return true;
}

void AVGMissionGimmickLever::OnInteractWith(AVGCharacterBase* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	Toggle();
}

void AVGMissionGimmickLever::Toggle()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (IsActivated())
	{
		SetStateTag(VigilantMissionTags::GimmickInactive);
	}
	else
	{
		SetStateTag(VigilantMissionTags::GimmickActive);
		ReportConditionMet();
	}
}

void AVGMissionGimmickLever::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
}

// Called when the game starts or when spawned
void AVGMissionGimmickLever::BeginPlay()
{
	Super::BeginPlay();
	
}

