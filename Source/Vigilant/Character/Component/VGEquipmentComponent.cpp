// Fill out your copyright notice in the Description page of Project Settings.

#include "VGEquipmentComponent.h"
#include "Equipment/VGEquippableActor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Common/VGGameplayTags.h"
#include "Kismet/GameplayStatics.h" 
#include "DrawDebugHelpers.h"

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

void UVGEquipmentComponent::Interact()
{
	//특별한 클래스가 아닌 공통 Pawn 클래스 사용
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	// 내 캐릭터(로컬)가 아니면 리턴
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;
	
	//마찬가지로 카메라 매니저 사용
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController || !PlayerController->PlayerCameraManager) return;

	
	FVector StartLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	FVector ForwardVector = PlayerController->PlayerCameraManager->GetActorForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * 1500.0f);

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(OwnerPawn);

    FCollisionShape SphereShape = FCollisionShape::MakeSphere(30.0f);

    bool bHit = GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::Identity, ECC_Visibility, SphereShape, CollisionParams);

    // 디버그 그리기
    DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f);
    if (bHit)
        DrawDebugSphere(GetWorld(), HitResult.Location, 30.0f, 16, FColor::Green, false, 2.0f);
    else
        DrawDebugSphere(GetWorld(), EndLocation, 30.0f, 16, FColor::Red, false, 2.0f);

    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();
        if (AVGEquippableActor* EquippableItem = Cast<AVGEquippableActor>(HitActor))
        {
            EVGEquipmentType TypeToEquip = EVGEquipmentType::Weapon;
            if (EquippableItem->GetName().Contains("Mission"))
            {
                TypeToEquip = EVGEquipmentType::MissionItem;
            }

            Server_EquipItem(EquippableItem, TypeToEquip);

            if (TypeToEquip == EVGEquipmentType::Weapon)
            {
                ActiveEquipmentSlot = EVGEquipmentSlot::RightHand;
                UE_LOG(LogTemp, Warning, TEXT("무기 장착 활성화 슬롯이 [오른손]으로 강제 전환"));
                OnEquipmentSlotChanged.Broadcast(ActiveEquipmentSlot);
            }
            else if (TypeToEquip == EVGEquipmentType::Shield)
            {
                ActiveEquipmentSlot = EVGEquipmentSlot::LeftHand;
                UE_LOG(LogTemp, Warning, TEXT("방패 장착 활성화 슬롯이 [왼손]으로 강제 전환"));
                OnEquipmentSlotChanged.Broadcast(ActiveEquipmentSlot);
            }
        }
    }
}

void UVGEquipmentComponent::DropItem()
{
	Server_DropItem(ActiveEquipmentSlot);
	UE_LOG(LogTemp, Log, TEXT("현재 활성화된 슬롯의 아이템 버리기"));
}

void UVGEquipmentComponent::SelectSlot(float SlotNumber)
{
	if (FMath::IsNearlyEqual(SlotNumber, 1.0f))
	{
		ActiveEquipmentSlot = EVGEquipmentSlot::LeftHand;
		UE_LOG(LogTemp, Warning, TEXT("왼손 슬롯 활성화"));
		OnEquipmentSlotChanged.Broadcast(ActiveEquipmentSlot);
	}
	else if (FMath::IsNearlyEqual(SlotNumber, 2.0f))
	{
		ActiveEquipmentSlot = EVGEquipmentSlot::RightHand;
		UE_LOG(LogTemp, Warning, TEXT("오른손 슬롯 활성화"));
		OnEquipmentSlotChanged.Broadcast(ActiveEquipmentSlot);
	}
}

void UVGEquipmentComponent::Server_EquipItem_Implementation(AVGEquippableActor* ItemToEquip, EVGEquipmentType ItemType)
{
	if (ItemToEquip == nullptr) return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter == nullptr || OwnerCharacter->GetMesh() == nullptr) return;
	
	// 아이템의 루트 컴포넌트 물리 끄기
	if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(ItemToEquip->GetRootComponent()))
        {
            RootComp->SetSimulatePhysics(false);
            RootComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 손에 쥔 무기에 캐릭터가 부딪히지 않게
        }
        
	// 기획 규칙에 따라 슬롯 판별 및 장착
	switch (ItemType)
	{
		case EVGEquipmentType::Weapon:
			// 오른손이 비어있고, 양손 무기를 들고 있지 않으면 장착 가능
			if (RightHandItem == nullptr && !EquipmentTags.HasTag(VigilantEquipmentTags::Weapon_TwoHand))
			{
				RightHandItem = ItemToEquip;
				
				// 오른손 소켓에 아이템 붙이기
				ItemToEquip->AttachToComponent(
					OwnerCharacter->GetMesh(), 
					FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
					FName("RightHandSocket")
				);
				
				EquipmentTags.AddTag(VigilantEquipmentTags::Weapon_OneHand);
				UE_LOG(LogTemp, Log, TEXT("서버: 오른손에 무기 장착 및 태그 추가 완료"));
				
				OnItemEquipped.Broadcast(EVGEquipmentSlot::RightHand, ItemToEquip);
			}
			break;

		case EVGEquipmentType::Shield:
			// 왼손이 비어있고, 양손 무기를 들고 있지 않으면 장착 가능
			if (LeftHandItem == nullptr && !EquipmentTags.HasTag(VigilantEquipmentTags::Weapon_TwoHand))
			{
				LeftHandItem = ItemToEquip;
				
				// 왼손 소켓에 아이템 붙이기
				ItemToEquip->AttachToComponent(
					OwnerCharacter->GetMesh(), 
					FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
					FName("LeftHandSocket")
				);
				
				EquipmentTags.AddTag(VigilantEquipmentTags::Weapon_Shield);
				UE_LOG(LogTemp, Log, TEXT("서버: 왼손에 방패 장착 및 태그 추가 완료"));
				
				OnItemEquipped.Broadcast(EVGEquipmentSlot::LeftHand, ItemToEquip);
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
				
				EquipmentTags.AddTag(VigilantEquipmentTags::Weapon_TwoHand);
				UE_LOG(LogTemp, Log, TEXT("서버: 양손 무기 장착 및 태그 추가 완료"));
				
				OnItemEquipped.Broadcast(EVGEquipmentSlot::RightHand, ItemToEquip);
			}
			break;

		case EVGEquipmentType::MissionItem:
			if (LeftHandItem == nullptr && !EquipmentTags.HasTag(VigilantEquipmentTags::Weapon_TwoHand))
			{
				LeftHandItem = ItemToEquip;
				ItemToEquip->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("LeftHandSocket"));
				// [Fix] 미션 아이템 장착 시에도 태그 추가 — 드롭 시 RemoveTag와 대응 필요
				EquipmentTags.AddTag(VigilantEquipmentTags::Item_Mission);
				UE_LOG(LogTemp, Log, TEXT("서버: 빈 왼손에 미션 아이템 장착 완료"));
				
				OnItemEquipped.Broadcast(EVGEquipmentSlot::LeftHand, ItemToEquip);
			}
			else if (RightHandItem == nullptr && !EquipmentTags.HasTag(VigilantEquipmentTags::Weapon_TwoHand))
			{
				RightHandItem = ItemToEquip;
				ItemToEquip->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RightHandSocket"));
				// [Fix] 미션 아이템 장착 시에도 태그 추가
				EquipmentTags.AddTag(VigilantEquipmentTags::Item_Mission);
				UE_LOG(LogTemp, Log, TEXT("서버: 빈 오른손에 미션 아이템 장착 완료"));
				
				OnItemEquipped.Broadcast(EVGEquipmentSlot::RightHand, ItemToEquip);
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
	
	// 아이템 분리
	TargetItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(TargetItem->GetRootComponent()))
	{
		RootComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RootComp->SetSimulatePhysics(true);
	}
	OnItemDropped.Broadcast(SlotToDrop);
	
	// 버리려는 아이템이 양손 무기면
	if (EquipmentTags.HasTag(VigilantEquipmentTags::Weapon_TwoHand) && (LeftHandItem == RightHandItem))
	{
		// 태그 제거
		EquipmentTags.RemoveTag(VigilantEquipmentTags::Weapon_TwoHand);
        
		// 양쪽 슬롯을 동시에 비움
		LeftHandItem = nullptr;
		RightHandItem = nullptr;
        
		UE_LOG(LogTemp, Log, TEXT("서버: 양손 무기 버리기 완료 (양손 슬롯 해제)"));
	}
	// [Fix] 왼손은 Shield/MissionItem, 오른손은 OneHand/MissionItem — 슬롯별 올바른 태그 제거
	else
	{
		if (SlotToDrop == EVGEquipmentSlot::LeftHand)
		{
			EquipmentTags.RemoveTag(VigilantEquipmentTags::Weapon_Shield);
			EquipmentTags.RemoveTag(VigilantEquipmentTags::Item_Mission);
			LeftHandItem = nullptr;
		}
		else
		{
			EquipmentTags.RemoveTag(VigilantEquipmentTags::Weapon_OneHand);
			EquipmentTags.RemoveTag(VigilantEquipmentTags::Item_Mission);
			RightHandItem = nullptr;
		}
		UE_LOG(LogTemp, Log, TEXT("서버: 한손 아이템 버리기 완료"));
	}
}
