#include "Equipment/VGEquippableActor.h"

#include "Character/VGCharacterBase.h"
#include "Character/Component/VGEquipmentComponent.h"

AVGEquippableActor::AVGEquippableActor()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AVGEquippableActor::OnInteractWith(AVGCharacterBase* Interactor)
{
	Super::OnInteractWith(Interactor);
	
	if (Interactor)
	{
		if (UVGEquipmentComponent* EquipComp = Interactor->FindComponentByClass<UVGEquipmentComponent>())
		{
			EVGEquipmentType TempType = EVGEquipmentType::Weapon;
			if (GetName().Contains("Mission"))
			{
				TempType = EVGEquipmentType::MissionItem;
			}
			
			EquipComp->Server_EquipItem(this, TempType);
		}
	}
}
