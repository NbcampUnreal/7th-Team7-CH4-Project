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
	if (EquipComp->LeftHandItem)
	{
		AVGMissionItemBase* LeftItem =
		Cast<AVGMissionItemBase>(EquipComp->LeftHandItem);
		if (LeftItem && LeftItem->EquipmentData)
		{
			if (UVGMissionItemDataAsset* ItemData 
				= Cast<UVGMissionItemDataAsset>(LeftItem->EquipmentData))
			{
				if (ItemData->ItemTypeTag == RequiredItemTypeTag)
				{
					UE_LOG(LogTemp, Log, TEXT("[%s] Found Item - %s"), *GetName(),
						*LeftItem->GetName());
					return true;
				}
			}
		}
	}
	
	if(EquipComp->RightHandItem)
	{
		AVGMissionItemBase* RightItem =
		Cast<AVGMissionItemBase>(EquipComp->RightHandItem);
		if (RightItem && RightItem->EquipmentData)
		{
			if (UVGMissionItemDataAsset* ItemData 
				= Cast<UVGMissionItemDataAsset>(RightItem->EquipmentData))
			{
				if (ItemData->ItemTypeTag == RequiredItemTypeTag)
				{
					UE_LOG(LogTemp, Log, TEXT("[%s] Found Item - %s"), *GetName(),
						*RightItem->GetName());
					return true;
				}
			}
		}
	}
	
	return false;
}

bool AVGMissionGimmickAltar::CanInteractWith(AActor* Interactor) const
{
	if (GimmickStateTag != VigilantMissionTags::GimmickInactive)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Now %s"), *GetName(), *GimmickStateTag.ToString());
		return false;
	}

	UVGEquipmentComponent* EquipComp =
		Interactor->FindComponentByClass<UVGEquipmentComponent>();
	if (!EquipComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] EquipComp is missing."), *GetName());
		return false;
	}
	// 빈 슬롯 중 인터랙터 보유 아이템과 매칭되는 것이 있는지 확인
	for (const FVGAltarPlacementSlot& Slot : PlacementSlots)
	{
		if (Slot.IsOccupied())
		{
			continue;
		}
		
		if (HasMatchingItemInHands(EquipComp, Slot.RequiredItemTypeTag))
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] Found required item."), *GetName());
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("[%s] No matching item."), *GetName());
	return false;
}

void AVGMissionGimmickAltar::OnInteractWith(AActor* Interactor, const FTransform& InteractTransform)
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
	
	// 매칭되는 빈 슬롯 찾아서 채우기
	for (FVGAltarPlacementSlot& Slot : PlacementSlots)
	{
		if (Slot.IsOccupied())
		{
			continue;
		}
		
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
		if (!CarryItem || !CarryItem->EquipmentData)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] This item isn't Carry"),*GetName());
			continue;
		}
		
		UVGMissionItemDataAsset* ItemData = Cast<UVGMissionItemDataAsset>(CarryItem->EquipmentData);
		if (!ItemData || ItemData->ItemTypeTag != Slot.RequiredItemTypeTag)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] No itemData or No RequiredItemTypeTag"),*GetName());
			continue;
		}

		EquipComp->Server_DropItem(HandSlot);
		CarryItem->PlaceOnTarget(this, Slot.AttachOffset);

		Slot.PlacedItem = CarryItem;
		
		UE_LOG(LogTemp, Warning, TEXT("[%s] Attach %s at %s RelativeLocation"),
			*GetName(), *CarryItem->GetName(), *Slot.AttachOffset.ToString());

		return true;
	}
	return false;
}

bool AVGMissionGimmickAltar::AreAllSlotsFilled()
{
	for (const FVGAltarPlacementSlot& Slot : PlacementSlots)
	{
		if (!Slot.IsOccupied())
		{
			return false;
		}
	}
	return true;
}

