#include "Equipment/VGEquippableActor.h"

#include "Character/VGCharacterBase.h"
#include "Character/Component/VGEquipmentComponent.h"

AVGEquippableActor::AVGEquippableActor()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AVGEquippableActor::OnInteractWith(AActor* Interactor, const FTransform& InteractTransform)
{
	Super::OnInteractWith(Interactor, InteractTransform);
	
	if (Interactor)
	{
		if (UVGEquipmentComponent* EquipComp = Interactor->FindComponentByClass<UVGEquipmentComponent>())
		{
			EquipComp->Server_EquipItem(this);
		}
	}
}
