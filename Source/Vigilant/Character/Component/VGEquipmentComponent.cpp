// Fill out your copyright notice in the Description page of Project Settings.

#include "VGEquipmentComponent.h"
#include "Equipment/VGEquippableActor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Common/VGGameplayTags.h"

UVGEquipmentComponent::UVGEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
}

void UVGEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UVGEquipmentComponent, LeftHandItem);
	DOREPLIFETIME(UVGEquipmentComponent, RightHandItem);
	DOREPLIFETIME(UVGEquipmentComponent, EquipmentTags);
}

void UVGEquipmentComponent::Server_EquipItem_Implementation(AVGEquippableActor* ItemToEquip, EVGEquipmentType ItemType)
{
	if (ItemToEquip == nullptr) return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter == nullptr || OwnerCharacter->GetMesh() == nullptr) return;
	
	// 기획 규칙에 따라 슬롯 판별 및 장착
	switch (ItemType)
	{
		case EVGEquipmentType::Weapon:
			// 오른손이 비어있고, 양손 무기를 들고 있지 않으면 장착 가능
			if (RightHandItem == nullptr && !EquipmentTags.HasTag(VigilantEquipmentTags::Equipped_State_TwoHanded))
			{
				RightHandItem = ItemToEquip;
				
				// 오른손 소켓에 아이템 붙이기
				ItemToEquip->AttachToComponent(
					OwnerCharacter->GetMesh(), 
					FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
					FName("RightHandSocket")
				);
				
				EquipmentTags.AddTag(VigilantEquipmentTags::Equipped_State_Weapon);
				UE_LOG(LogTemp, Log, TEXT("서버: 오른손에 무기 장착 및 태그 추가 완료"));
			}
			break;

		case EVGEquipmentType::Shield:
			// 왼손이 비어있고, 양손 무기를 들고 있지 않으면 장착 가능
			if (LeftHandItem == nullptr && !EquipmentTags.HasTag(VigilantEquipmentTags::Equipped_State_TwoHanded))
			{
				LeftHandItem = ItemToEquip;
				
				// 왼손 소켓에 아이템 붙이기
				ItemToEquip->AttachToComponent(
					OwnerCharacter->GetMesh(), 
					FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
					FName("LeftHandSocket")
				);
				
				EquipmentTags.AddTag(VigilantEquipmentTags::Equipped_State_Shield);
				UE_LOG(LogTemp, Log, TEXT("서버: 왼손에 방패 장착 및 태그 추가 완료"));
			}
			break;
		
		case EVGEquipmentType::TwoHandedWeapon:
			// 양손이 모두 비어있어야 장착 가능
			if (LeftHandItem == nullptr && RightHandItem == nullptr)
			{
				// 왼손,오른손 소켓에 장착
				LeftHandItem = ItemToEquip;
				RightHandItem = ItemToEquip;
				
				// 시각적으로는 주 손(오른손) 소켓에 부착
				// 왼손은 애니메이션의 IK나 전용 포즈를 통해 무기에 고정되도록 처리
				ItemToEquip->AttachToComponent(
				OwnerCharacter->GetMesh(), 
				FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
				FName("RightHandSocket")
				);
				
				EquipmentTags.AddTag(VigilantEquipmentTags::Equipped_State_TwoHanded);
				UE_LOG(LogTemp, Log, TEXT("서버: 양손 무기 장착 및 태그 추가 완료"));
			}
			break;

		case EVGEquipmentType::MissionItem:
			if (LeftHandItem == nullptr && !EquipmentTags.HasTag(VigilantEquipmentTags::Equipped_State_TwoHanded))
			{
				LeftHandItem = ItemToEquip;
				ItemToEquip->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("LeftHandSocket"));
				UE_LOG(LogTemp, Log, TEXT("서버: 빈 왼손에 미션 아이템 장착 완료"));
			}
			else if (RightHandItem == nullptr && !EquipmentTags.HasTag(VigilantEquipmentTags::Equipped_State_TwoHanded))
			{
				RightHandItem = ItemToEquip;
				ItemToEquip->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RightHandSocket"));
				UE_LOG(LogTemp, Log, TEXT("서버: 빈 오른손에 미션 아이템 장착 완료"));
			}
			break;
	}
}

void UVGEquipmentComponent::Server_DropItem_Implementation(EVGEquipmentSlot SlotToDrop)
{
	// 버릴 대상 아이템을 임시로 담아둘 변수
	AVGEquippableActor* TargetItem = nullptr;
	
	// 선택한 슬롯에서 아이템 확인
	if (SlotToDrop == EVGEquipmentSlot::LeftHand) TargetItem = LeftHandItem;
	else if (SlotToDrop == EVGEquipmentSlot::RightHand) TargetItem = RightHandItem;

	if (TargetItem == nullptr) return;
	
	// 버리려는 아이템이 양손 무기면
	if (EquipmentTags.HasTag(VigilantEquipmentTags::Equipped_State_TwoHanded) && (LeftHandItem == RightHandItem))
	{
		// 소켓 분리 및 태그 제거
		TargetItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		EquipmentTags.RemoveTag(VigilantEquipmentTags::Equipped_State_TwoHanded);
        
		// 양쪽 슬롯을 동시에 비움
		LeftHandItem = nullptr;
		RightHandItem = nullptr;
        
		UE_LOG(LogTemp, Log, TEXT("서버: 양손 무기 버리기 완료 (양손 슬롯 해제)"));
	}
	// 한손 아이템(무기, 방패 등)일 경우 
	else
	{
		TargetItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        
		if (SlotToDrop == EVGEquipmentSlot::LeftHand)
		{
			EquipmentTags.RemoveTag(VigilantEquipmentTags::Equipped_State_Weapon);
			EquipmentTags.RemoveTag(VigilantEquipmentTags::Equipped_State_MissionItem);
			LeftHandItem = nullptr;
		}
		else
		{
			EquipmentTags.RemoveTag(VigilantEquipmentTags::Equipped_State_Weapon);
			EquipmentTags.RemoveTag(VigilantEquipmentTags::Equipped_State_MissionItem);
			RightHandItem = nullptr;
		}
		UE_LOG(LogTemp, Log, TEXT("서버: 한손 아이템 버리기 완료"));
	}
}
