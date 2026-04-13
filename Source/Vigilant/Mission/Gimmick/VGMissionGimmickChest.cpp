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
	
	// [Fix] Interactor null 체크 추가
	if (!Interactor)
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
	
	UE_LOG(LogTemp, Error, TEXT("[%s] You don't have required Item!"), *GetName());
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
		UE_LOG(LogTemp, Log, TEXT("[%s] Chest Open!"), *GetName());
		SetStateTag(VigilantMissionTags::GimmickCompleted);
		OnGimmickInteracted.Broadcast(this, Interactor);
	}
	
}

void AVGMissionGimmickChest::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
	
	if (GimmickStateTag == VigilantMissionTags::GimmickCompleted)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

bool AVGMissionGimmickChest::TryUseItemFromSlot(UVGEquipmentComponent* EquipComp, AVGEquippableActor* SlotItem, EVGEquipmentSlot Slot)
{
	AVGMissionItemBase* MissionItem = Cast<AVGMissionItemBase>(SlotItem);
	if (!MissionItem)
	{
		return false;
	}
	
	if (!MissionItem->EquipmentData)
	{
		return false;
	}
	
	UVGMissionItemDataAsset* ItemData = Cast<UVGMissionItemDataAsset>(MissionItem->EquipmentData);
	if (!ItemData ||ItemData->ItemTypeTag != RequiredItemTypeTag)
	{
		return false;
	}
	// 아이템 사용 처리
	MissionItem->SetStateTag(VigilantMissionTags::ItemUsed);
	EquipComp->Server_DropItem(Slot);
	return true;
}
