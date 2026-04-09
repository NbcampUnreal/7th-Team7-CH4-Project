#include "VGMissionGimmickAltar.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Mission/Item/VGMissionItemCarry.h"
#include "Mission/Item/VGMissionItemBase.h"
#include "Data/VGMissionItemDataAsset.h"

AVGMissionGimmickAltar::AVGMissionGimmickAltar()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GimmickTypeTag = VigilantMissionTags::AltarGimmick;
}

bool AVGMissionGimmickAltar::HasMatchingItemInHands(UVGEquipmentComponent* EquipComp, FGameplayTag RequiredItemTypeTag) const
{
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
	// 빈 슬롯 중 인터랙터 보유 아이템과 매칭되는 것이 있는지 확인
	for (const FVGAltarPlacementSlot& Slot : PlacementSlots)
	{
		if (Slot.IsOccupied()) continue;

		if (HasMatchingItemInHands(EquipComp, Slot.RequiredItemTypeTag))
			return true;
	}

	return false;
}

void AVGMissionGimmickAltar::OnInteractWith(AVGCharacterBase* Interactor)
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
	
	UVGEquipmentComponent* EquipComp = Interactor->FindComponentByClass<UVGEquipmentComponent>();
	if (!EquipComp)
	{
		return;
	}
	
	// 매칭되는 빈 슬롯 찾아서 채우기
	for (FVGAltarPlacementSlot& Slot : PlacementSlots)
	{
		if (Slot.IsOccupied()) continue;

		bool bPlaced = TryPlaceItemToSlot(EquipComp, Slot);
		if (bPlaced)
		{
			OnGimmickInteracted.Broadcast(this, Interactor);
			break; // 한 번에 하나씩
		}
	}
	
	// 모든 슬롯이 채워졌으면 완료
	if (AreAllSlotsFilled())
	{
		SetStateTag(VigilantMissionTags::GimmickCompleted);
	}
}

bool AVGMissionGimmickAltar::TryPlaceItemToSlot(UVGEquipmentComponent* EquipComp, FVGAltarPlacementSlot& Slot)
{
	for (EVGEquipmentSlot HandSlot : 
		 { EVGEquipmentSlot::LeftHand, EVGEquipmentSlot::RightHand })
	{
		AVGEquippableActor* HandItem = (HandSlot == EVGEquipmentSlot::LeftHand)
			? EquipComp->LeftHandItem : EquipComp->RightHandItem;
		if (!HandItem)
		{
			continue;
		}
		
		AVGMissionItemCarry* CarryItem = Cast<AVGMissionItemCarry>(HandItem);
		if (!CarryItem || !CarryItem->ItemDataAsset) continue;
		if (CarryItem->ItemDataAsset->ItemTypeTag != Slot.RequiredItemTypeTag) continue;

		EquipComp->Server_DropItem(HandSlot);
		CarryItem->PlaceOnTarget(this);

		// 소켓 또는 오프셋으로 위치 보정
		if (Slot.AttachSocketName != NAME_None)
		{
			CarryItem->AttachToComponent(
				GetRootComponent(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				Slot.AttachSocketName);
		}
		else
		{
			CarryItem->SetActorRelativeLocation(Slot.AttachOffset);
		}

		Slot.PlacedItem = CarryItem;
		return true;
	}
	return false;
}

bool AVGMissionGimmickAltar::AreAllSlotsFilled()
{
	for (const FVGAltarPlacementSlot& Slot : PlacementSlots)
	{
		if (!Slot.IsOccupied()) return false;
	}
	return true;
}

