#include "Equipment/VGEquippableActor.h"

AVGEquippableActor::AVGEquippableActor()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AVGEquippableActor::OnInteractWith(AVGCharacterBase* Interactor)
{
	Super::OnInteractWith(Interactor);
	
	// TODO: E키가 눌리면 캐릭터의 EquipmentComponent에게 이 아이템을 줍도록 지시
}
