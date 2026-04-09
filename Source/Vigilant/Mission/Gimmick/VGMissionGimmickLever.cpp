#include "VGMissionGimmickLever.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"
#include "Character/Component/VGEquipmentComponent.h"

AVGMissionGimmickLever::AVGMissionGimmickLever()
{
	PrimaryActorTick.bCanEverTick = false;
	GimmickTypeTag = VigilantMissionTags::LeverGimmick;
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
		if (UVGEquipmentComponent* EquipComp =
			Interactor->FindComponentByClass<UVGEquipmentComponent>())
		{
			EquipComp->Server_InteractWithActor(this, Interactor);
		}
		return;
	}
	
	if (!CanInteractWith(Interactor))
	{
		return;
	}
	
	Toggle();
	OnGimmickInteracted.Broadcast(this, Interactor);
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
	}
}

void AVGMissionGimmickLever::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
}
