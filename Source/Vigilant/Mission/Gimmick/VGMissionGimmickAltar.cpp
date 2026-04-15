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

bool AVGMissionGimmickAltar::CanInteractWith(AActor* Interactor) const
{
	if (GimmickStateTag != VigilantMissionTags::GimmickInactive)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Now %s"), *GetName(), *GimmickStateTag.ToString());
		return false;
	}
	
	if (!Interactor)
	{
		return false;
	}
    	
	UVGEquipmentComponent* EquipComp =
		Interactor->FindComponentByClass<UVGEquipmentComponent>();
	if (!EquipComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] EquipComp is missing."), *GetName());
		return false;
	}
	
	// 비어있는 슬롯 중 인터랙터 보유 아이템과 매칭되는 것이 있는지 확인
	for (const FVGAltarPlacementSlot& Slot : PlacementSlots)
	{
		if (Slot.IsOccupied())
		{
			continue;
		}
		
		if (FindMissionItemByTag(EquipComp, Slot.RequiredItemTypeTag))
		{
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
		
		if (TryPlaceItemToSlot(EquipComp, Slot))
		{
			OnGimmickInteracted.Broadcast(this, Interactor);
			break;
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
	// 헬퍼로 일치하는 아이템을 찾은 뒤 CarryItem으로 캐스팅
	AVGMissionItemBase* FoundItem =
		FindMissionItemByTag(EquipComp, Slot.RequiredItemTypeTag);
	
	AVGMissionItemCarry* CarryItem = Cast<AVGMissionItemCarry>(FoundItem);
	if (!CarryItem)
	{
		return false;
	}
 
	// 어느 손에 들고 있는지 확인해서 드롭 슬롯 결정
	EVGEquipmentSlot HandSlot = (EquipComp->LeftHandItem == CarryItem)
		? EVGEquipmentSlot::LeftHand
		: EVGEquipmentSlot::RightHand;
 
	EquipComp->Server_DropItem(HandSlot);
	CarryItem->PlaceOnTarget(this, Slot.AttachOffset);
	Slot.PlacedItem = CarryItem;
 
	UE_LOG(LogTemp, Warning, TEXT("[%s] Attach %s at %s"),
		*GetName(), *CarryItem->GetName(), *Slot.AttachOffset.ToString());
 
	return true;
}

bool AVGMissionGimmickAltar::AreAllSlotsFilled() const
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

