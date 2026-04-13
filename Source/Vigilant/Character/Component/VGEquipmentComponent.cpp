#include "VGEquipmentComponent.h"
#include "Equipment/VGEquippableActor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Common/VGGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Character/VGCharacterBase.h"
#include "Data/VGEquipmentDataAsset.h"
#include "Components/MeshComponent.h"
#include "Engine/OverlapResult.h"

UVGEquipmentComponent::UVGEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UVGEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 로컬 플레이어인 경우에만 0.1초마다 주변 아이템 스캔
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_UpdateInteractable, 
			this, 
			&UVGEquipmentComponent::UpdateInteractableTarget, 
			InteractionCheckInterval, 
			true
		);
	}
}

void UVGEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVGEquipmentComponent, LeftHandItem);
	DOREPLIFETIME(UVGEquipmentComponent, RightHandItem);
}


void UVGEquipmentComponent::Server_InteractWithActor_Implementation(AActor* TargetActor, AActor* Interactor, const FTransform& InteractTransform)
{
	if (!TargetActor || !Interactor)
	{
		return;
	}
    
	// EquipmentComponent는 GimmickBase를 모른다
	// IVGInteractable 인터페이스만 안다
	if (TargetActor->Implements<UVGInteractable>())
	{
		if (IVGInteractable::Execute_CanInteract(TargetActor, Interactor))
		{
			IVGInteractable::Execute_OnInteract(TargetActor, Interactor, InteractTransform);
		}
	}
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

	// 하이라이트(가장 가까운) 된 타겟에게 상호작용 서버 요청
	if (CurrentInteractableTarget && CurrentInteractableTarget->Implements<UVGInteractable>())
	{
		Server_InteractWithActor(CurrentInteractableTarget, OwnerCharacter, CurrentInteractableTarget->GetActorTransform());
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

void UVGEquipmentComponent::UpdateInteractableTarget()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	AVGCharacterBase* OwnerCharacter = Cast<AVGCharacterBase>(OwnerPawn);
	if (!OwnerCharacter) return;

	FVector SearchCenter = OwnerPawn->GetActorLocation();
	float SearchRadius = 250.0f;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);

	TArray<FOverlapResult> OverlapResults;
	GetWorld()->OverlapMultiByChannel(
	   OverlapResults, SearchCenter, FQuat::Identity, ECC_Visibility,
	   FCollisionShape::MakeSphere(SearchRadius), QueryParams
	);

	AActor* ClosestTarget = nullptr;
	float MinDistanceSquared = MAX_flt;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
    
		// 1차: 상호작용 인터페이스가 있는지 확인
		if (HitActor && HitActor->Implements<UVGInteractable>())
		{
			bool bIsValidTarget = false;

			// 상대방이 플레이어(캐릭터)인 경우 
			if (HitActor->IsA<AVGCharacterBase>()) 
			{
				// 막고라 아이템을 들고 있는지 검사 (태그나 클래스로 확인)
				bool bHasMakgoraItem = false;
            
				// 양손 무기 중 막고라 전용 태그나 이름이 있는지 확인
				if (RightHandItem && RightHandItem->ActorHasTag(FName("MakgoraItem"))) bHasMakgoraItem = true;
				if (LeftHandItem && LeftHandItem->ActorHasTag(FName("MakgoraItem"))) bHasMakgoraItem = true;

				// 막고라 아이템을 쥐고 있을 때만 타겟으로 인정
				if (bHasMakgoraItem)
				{
					bIsValidTarget = true;
				}
			}
			// 일반 상호작용 오브젝트인 경우
			else if (HitActor->ActorHasTag(FName("InteractTarget")))
			{
				bIsValidTarget = true;
			}

			// 타겟으로 합격한 녀석들만 거리 계산 진행
			if (bIsValidTarget)
			{
				if (IVGInteractable::Execute_CanInteract(HitActor, OwnerCharacter))
				{
					float DistSquared = FVector::DistSquared(SearchCenter, HitActor->GetActorLocation());
					if (DistSquared < MinDistanceSquared)
					{
						MinDistanceSquared = DistSquared;
						ClosestTarget = HitActor;
					}
				}
			}
		}
	}

	if (ClosestTarget != CurrentInteractableTarget)
	{
		if (CurrentInteractableTarget) SetHighlight(CurrentInteractableTarget, false);
		if (ClosestTarget) SetHighlight(ClosestTarget, true);
		CurrentInteractableTarget = ClosestTarget;
	}
}

void UVGEquipmentComponent::SetHighlight(AActor* TargetActor, bool bHighlight)
{
	if (!TargetActor) return;

	TArray<UMeshComponent*> MeshComps;
	TargetActor->GetComponents<UMeshComponent>(MeshComps);

	// 액터 안에 Highlight라는 컴포넌트 태그를 가진 메쉬가 있는지 검사
	bool bHasHighlightTag = false;
	for (UMeshComponent* Mesh : MeshComps)
	{
		if (Mesh->ComponentHasTag(FName("Highlight")))
		{
			bHasHighlightTag = true;
			break;
		}
	}

	// 상황에 맞게 커스텀 뎁스(외곽선) 적용
	for (UMeshComponent* Mesh : MeshComps)
	{
		// 특정 부위만 빛나게 세팅된 액터라면 
		if (bHasHighlightTag)
		{
			if (Mesh->ComponentHasTag(FName("Highlight")))
			{
				// 태그가 붙은 녀석만 외곽선 켜기/끄기 (문짝)
				Mesh->SetRenderCustomDepth(bHighlight);
				Mesh->SetCustomDepthStencilValue(1);
			}
			else
			{
				// 태그가 없는 녀석은 외곽선 끄기 (문틀)
				Mesh->SetRenderCustomDepth(false); 
			}
		}
		// 태그 세팅이 없는 일반 액터라면
		else
		{
			// 기존처럼 액터 전체 메쉬 다 켜기/끄기
			Mesh->SetRenderCustomDepth(bHighlight);
			Mesh->SetCustomDepthStencilValue(1);
		}
	}
}
