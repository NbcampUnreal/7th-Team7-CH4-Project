#include "VGMissionGimmickAltar.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Mission/Item/VGMissionItemCarry.h"
#include "Data/VGMissionItemDataAsset.h"

AVGMissionGimmickAltar::AVGMissionGimmickAltar()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AVGMissionGimmickAltar::CanInteractWith(AVGCharacterBase* Interactor) const
{
	// 이미 아이템이 놓여있으면 불가
	if (GimmickStateTag != VigilantMissionTags::GimmickInactive)
	{
		return false;
	}

	UVGEquipmentComponent* EquipComp =
		Interactor->FindComponentByClass<UVGEquipmentComponent>();
	if (!EquipComp)
	{
		return false;
	}
	
	AVGMissionItemCarry* LeftItem =
		Cast<AVGMissionItemCarry>(EquipComp->LeftHandItem);
	if (LeftItem && LeftItem->ItemDataAsset &&
		LeftItem->ItemDataAsset->ItemTypeTag == RequiredItemTypeTag)
	{
		return true;
	}

	AVGMissionItemCarry* RightItem =
		Cast<AVGMissionItemCarry>(EquipComp->RightHandItem);
	if (RightItem && RightItem->ItemDataAsset &&
		RightItem->ItemDataAsset->ItemTypeTag == RequiredItemTypeTag)
	{
		return true;
	}

	return false;
}

void AVGMissionGimmickAltar::OnInteractWith(AVGCharacterBase* Interactor)
{
	
	if (!HasAuthority())
	{
		return;
	}
	
	if (!CanInteractWith(Interactor))
	{
		return;
	}
	
	UVGEquipmentComponent* EquipComp = Interactor->FindComponentByClass<UVGEquipmentComponent>();
	if (!EquipComp)
	{
		return;
	}
	
	bool bPlaced= TryPlaceItemFromSlot(
		EquipComp, EquipComp->LeftHandItem, EVGEquipmentSlot::LeftHand);
	
	if (!bPlaced)
	{
		TryPlaceItemFromSlot(
			EquipComp, EquipComp->RightHandItem, EVGEquipmentSlot::RightHand);
	}
}

bool AVGMissionGimmickAltar::TryPlaceItemFromSlot(UVGEquipmentComponent* EquipComp, AVGEquippableActor* SlotItem,
	EVGEquipmentSlot Slot)
{
	AVGMissionItemCarry* CarryItem = Cast<AVGMissionItemCarry>(SlotItem);
	if (!CarryItem)
	{
		return false;
	}
	
	if (!CarryItem->ItemDataAsset)
	{
		return false;
	}
	
	if (CarryItem->ItemDataAsset->ItemTypeTag != RequiredItemTypeTag)
	{
		return false;
	}

	// EquipComponent에서 분리
	EquipComp->Server_DropItem(Slot);

	// 제단에 부착
	CarryItem->PlaceOnTarget(this);
	PlacedItem = CarryItem;

	return true;
}

