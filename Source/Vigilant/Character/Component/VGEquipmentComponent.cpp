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
	DOREPLIFETIME(UVGEquipmentComponent, EquipmentTags);
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
	}
	else if (OldItem)
	{
		HandleItemAttachment(OldItem, NAME_None, false);
	}
	
}

void UVGEquipmentComponent::OnRep_RighthandItem(AVGEquippableActor* OldItem)
{
	if (RightHandItem)
	{
		HandleItemAttachment(RightHandItem, RightHandItem->EquipmentData->RightHandSocketName, true);
	}
	else if (OldItem)
	{
		HandleItemAttachment(OldItem, NAME_None, false);
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
	if (!ItemToEquip)
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
	// 한손 아이템(무기, 방패 등)일 경우 
	else
	{
		if (SlotToDrop == EVGEquipmentSlot::LeftHand)
		{
			EquipmentTags.RemoveTag(VigilantEquipmentTags::Weapon_OneHand);
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
