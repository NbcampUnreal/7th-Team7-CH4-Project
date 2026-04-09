#include "VGEquipmentComponent.h"
#include "Equipment/VGEquippableActor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Common/VGGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Character/VGCharacterBase.h"
#include "Data/VGEquipmentDataAsset.h"

UVGEquipmentComponent::UVGEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
}

void UVGEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVGEquipmentComponent, LeftHandItem);
	DOREPLIFETIME(UVGEquipmentComponent, RightHandItem);
}

void UVGEquipmentComponent::Interact()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	AVGCharacterBase* OwnerCharacter = Cast<AVGCharacterBase>(OwnerPawn);
	if (!OwnerCharacter)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		return;
	}

	FVector StartLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	FVector ForwardVector = PlayerController->PlayerCameraManager->GetActorForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * 700.0f);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwnerPawn);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(30.0f);

	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::Identity, ECC_Visibility,
	                                             SphereShape, CollisionParams);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->Implements<UVGInteractable>())
		{
			if (IVGInteractable::Execute_CanInteract(HitActor, OwnerCharacter))
			{
				IVGInteractable::Execute_OnInteract(HitActor, OwnerCharacter);
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

void UVGEquipmentComponent::OnRep_LefthandItem(AVGEquippableActor* OldItem)
{
	if (LeftHandItem)
	{
		HandleItemAttachment(LeftHandItem, LeftHandItem->EquipmentData->LeftHandSocketName, true);
		OnItemEquipped.Broadcast(EVGEquipmentSlot::LeftHand, RightHandItem);
	}
	else if (OldItem)
	{
		HandleItemAttachment(OldItem, NAME_None, false);
		OnItemDropped.Broadcast(EVGEquipmentSlot::LeftHand);
	}
}

void UVGEquipmentComponent::OnRep_RighthandItem(AVGEquippableActor* OldItem)
{
	if (RightHandItem)
	{
		HandleItemAttachment(RightHandItem, RightHandItem->EquipmentData->RightHandSocketName, true);
		OnItemEquipped.Broadcast(EVGEquipmentSlot::RightHand, RightHandItem);
	}
	else if (OldItem)
	{
		HandleItemAttachment(OldItem, NAME_None, false);
		OnItemDropped.Broadcast(EVGEquipmentSlot::RightHand);
	}
}

void UVGEquipmentComponent::HandleItemAttachment(AVGEquippableActor* Item, FName SocketName, bool bIsEquipping)
{
	if (!Item)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return;
	}

	UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(Item->GetRootComponent());

	if (bIsEquipping)
	{
		if (RootComp)
		{
			RootComp->SetSimulatePhysics(false);
			RootComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		Item->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		                        SocketName);
	}
	else
	{
		Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		if (RootComp)
		{
			RootComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			RootComp->SetSimulatePhysics(true);
		}
	}
}

void UVGEquipmentComponent::Server_EquipItem_Implementation(AVGEquippableActor* ItemToEquip)
{
	if (!ItemToEquip || !ItemToEquip->EquipmentData)
	{
		return;
	}
	
	if (ItemToEquip->GetAttachParentActor() != nullptr)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return;
	}

	// --- Validation ---
	float Distance = FVector::Distance(OwnerCharacter->GetActorLocation(), ItemToEquip->GetActorLocation());
	float MaxInteractDistance = 300.0f;
	if (Distance > MaxInteractDistance)
	{
		return;
	}

	bool bEquipSuccess = false;
	UVGEquipmentDataAsset* ItemData = ItemToEquip->EquipmentData;

	switch (ItemData->EquipRule)
	{
	case EVGEquipRules::RightHandOnly:
		bEquipSuccess = TryEquipToRightHand(ItemToEquip);
		break;

	case EVGEquipRules::LeftHandOnly:
		bEquipSuccess = TryEquipToLeftHand(ItemToEquip);
		break;

	case EVGEquipRules::EitherHand:
		bEquipSuccess = TryEquipToEitherHand(ItemToEquip);
		break;

	case EVGEquipRules::BothHands:
		bEquipSuccess = TryEquipToBothHands(ItemToEquip);
		break;
	}

	if (bEquipSuccess)
	{
		// TODO: 캐릭터에 ItemData->GrantedEquipmentTag 할당
		EVGEquipmentSlot EquippedSlot = (ItemData->EquipRule == EVGEquipRules::LeftHandOnly)
			                                ? EVGEquipmentSlot::LeftHand
			                                : EVGEquipmentSlot::RightHand;
		OnItemEquipped.Broadcast(EquippedSlot, ItemToEquip);
	}
}

void UVGEquipmentComponent::Server_DropItem_Implementation(EVGEquipmentSlot SlotToDrop)
{
	AVGEquippableActor* TargetItem = nullptr;

	if (SlotToDrop == EVGEquipmentSlot::LeftHand)
	{
		TargetItem = LeftHandItem;
	}
	else if (SlotToDrop == EVGEquipmentSlot::RightHand)
	{
		TargetItem = RightHandItem;
	}

	if (TargetItem == nullptr)
	{
		return;
	}

	HandleItemAttachment(TargetItem, NAME_None, false);

	if (TargetItem->EquipmentData->EquipRule == EVGEquipRules::BothHands)
	{
		LeftHandItem = nullptr;
		RightHandItem = nullptr;
	}
	else
	{
		if (SlotToDrop == EVGEquipmentSlot::LeftHand)
		{
			LeftHandItem = nullptr;
		}
		else
		{
			RightHandItem = nullptr;
		}
	}

	OnItemDropped.Broadcast(SlotToDrop);
}


bool UVGEquipmentComponent::TryEquipToRightHand(AVGEquippableActor* ItemToEquip)
{
	if (RightHandItem == nullptr)
	{
		RightHandItem = ItemToEquip;
		HandleItemAttachment(RightHandItem, ItemToEquip->EquipmentData->RightHandSocketName, true);
		return true;
	}

	bool bIsCurrentItemSwappable = (RightHandItem->EquipmentData->EquipRule == EVGEquipRules::EitherHand);
	if (bIsCurrentItemSwappable && LeftHandItem == nullptr)
	{
		LeftHandItem = RightHandItem;
		HandleItemAttachment(LeftHandItem, LeftHandItem->EquipmentData->LeftHandSocketName, true);

		OnItemEquipped.Broadcast(EVGEquipmentSlot::LeftHand, LeftHandItem);
		
		RightHandItem = ItemToEquip;
		HandleItemAttachment(RightHandItem, ItemToEquip->EquipmentData->RightHandSocketName, true);

		return true;
	}

	return false;
}

bool UVGEquipmentComponent::TryEquipToLeftHand(AVGEquippableActor* ItemToEquip)
{
	if (LeftHandItem == nullptr)
	{
		LeftHandItem = ItemToEquip;
		HandleItemAttachment(LeftHandItem, ItemToEquip->EquipmentData->LeftHandSocketName, true);
		return true;
	}

	bool bIsCurrentItemSwappable = (LeftHandItem->EquipmentData->EquipRule == EVGEquipRules::EitherHand);
	if (bIsCurrentItemSwappable && RightHandItem == nullptr)
	{
		RightHandItem = LeftHandItem;
		HandleItemAttachment(RightHandItem, RightHandItem->EquipmentData->RightHandSocketName, true);

		OnItemEquipped.Broadcast(EVGEquipmentSlot::RightHand, RightHandItem);
		
		LeftHandItem = ItemToEquip;
		HandleItemAttachment(LeftHandItem, ItemToEquip->EquipmentData->LeftHandSocketName, true);
		return true;
	}

	return false;
}

bool UVGEquipmentComponent::TryEquipToEitherHand(AVGEquippableActor* ItemToEquip)
{
	// UX 일관성을 위해 오른손 우선 장착
	if (RightHandItem == nullptr)
	{
		RightHandItem = ItemToEquip;
		HandleItemAttachment(RightHandItem, ItemToEquip->EquipmentData->RightHandSocketName, true);
		return true;
	}
	else if (LeftHandItem == nullptr)
	{
		LeftHandItem = ItemToEquip;
		HandleItemAttachment(LeftHandItem, ItemToEquip->EquipmentData->LeftHandSocketName, true);
		return true;
	}

	return false;
}

bool UVGEquipmentComponent::TryEquipToBothHands(AVGEquippableActor* ItemToEquip)
{
	if (RightHandItem == nullptr && LeftHandItem == nullptr)
	{
		RightHandItem = ItemToEquip;
		LeftHandItem = ItemToEquip;
		HandleItemAttachment(ItemToEquip, ItemToEquip->EquipmentData->RightHandSocketName, true);
		return true;
	}

	return false;
}
