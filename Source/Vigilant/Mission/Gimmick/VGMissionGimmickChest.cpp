#include "VGMissionGimmickChest.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"
#include "Mission/Item/VGMissionItemBase.h"
#include "Data/VGMissionItemDataAsset.h"

AVGMissionGimmickChest::AVGMissionGimmickChest()
{
	PrimaryActorTick.bCanEverTick = false;
	GimmickTypeTag = VigilantMissionTags::ChestGimmick;
}

bool AVGMissionGimmickChest::CanInteractWith(AActor* Interactor) const
{
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
	
	// 왼손 또는 오른손에 필요한 아이템이 있는지 확인
	AVGMissionItemBase* LeftItem =
		Cast<AVGMissionItemBase>(EquipComp->LeftHandItem);
	if (LeftItem && LeftItem->ItemDataAsset &&
		LeftItem->ItemDataAsset->ItemTypeTag == RequiredItemTypeTag)
	{
		return true;
	}

	AVGMissionItemBase* RightItem =
		Cast<AVGMissionItemBase>(EquipComp->RightHandItem);
	if (RightItem && RightItem->ItemDataAsset &&
		RightItem->ItemDataAsset->ItemTypeTag == RequiredItemTypeTag)
	{
		return true;
	}

	return false;
}

void AVGMissionGimmickChest::OnInteractWith(AActor* Interactor, const FTransform& InteractTransform)
{
	if (!HasAuthority())
	{
		return;
	}
	if (!CanInteractWith(Interactor)) return;

	// 열쇠 아이템 찾아서 사용 처리
	UVGEquipmentComponent* EquipComp =
		Interactor->FindComponentByClass<UVGEquipmentComponent>();
	if (!EquipComp)
	{
		return;
	}
	
	// 왼손부터 확인, 없으면 오른손 확인
	bool bUsed = TryUseItemFromSlot(
		EquipComp, EquipComp->LeftHandItem, EVGEquipmentSlot::LeftHand);
	if (!bUsed)
	{
		bUsed = TryUseItemFromSlot(
		EquipComp, EquipComp->RightHandItem, EVGEquipmentSlot::RightHand);
	}

	if (bUsed)
	{
		SetStateTag(VigilantMissionTags::GimmickCompleted);
		OnGimmickInteracted.Broadcast(this, Interactor);
	}
	
}

bool AVGMissionGimmickChest::TryUseItemFromSlot(UVGEquipmentComponent* EquipComp, AVGEquippableActor* SlotItem, EVGEquipmentSlot Slot)
{
	AVGMissionItemBase* MissionItem = Cast<AVGMissionItemBase>(SlotItem);
	if (!MissionItem)
	{
		return false;
	}
	
	if (!MissionItem->ItemDataAsset)
	{
		return false;
	}
	
	if (MissionItem->ItemDataAsset->ItemTypeTag != RequiredItemTypeTag)
	{
		return false;
	}
	// 아이템 사용 처리
	MissionItem->SetStateTag(VigilantMissionTags::ItemUsed);
	EquipComp->Server_DropItem(Slot);
	return true;
}
