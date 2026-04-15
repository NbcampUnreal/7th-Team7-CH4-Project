#include "VGMissionGimmickAltar.h"

#include "NiagaraComponentPool.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Mission/Item/VGMissionItemCarry.h"
#include "Mission/Item/VGMissionItemBase.h"
#include "Data/VGMissionItemDataAsset.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

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
		
		UVGMissionItemDataAsset* ItemDataAsset = Slot.ItemDataAsset.Get();
		if (!ItemDataAsset)
		{
			continue;
		}
		
		if (FindMissionItemByTag(EquipComp, ItemDataAsset->ItemTypeTag))
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
		GetWorldTimerManager().ClearTimer(HintVisibilityTimerHandle);
		SetStateTag(VigilantMissionTags::GimmickCompleted);
	}
}

void AVGMissionGimmickAltar::UpdateHintEffectVisibility()
{
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC || !LocalPC->GetPawn())
	{
		return;
	}
	
	float DistSq = FVector::DistSquared(
		GetActorLocation(),
		LocalPC->GetPawn()->GetActorLocation()
	);
    
	bool bShouldShow = DistSq < FMath::Square(HintVisibleRange);
    
	for (int32 i = 0; i < HintEffectComponents.Num(); i++)
	{
		UNiagaraComponent* Comp = HintEffectComponents[i];
		if (!Comp)
		{
			continue;
		}
		
		// 이미 채워진 슬롯은 무조건 비활성
		bool bSlotEmpty = !PlacementSlots[i].IsOccupied();
		bool bActivate = bShouldShow && bSlotEmpty;
        
		if (bActivate && !Comp->IsActive())
		{
			Comp->Activate();
		}
		else if (!bActivate && Comp->IsActive())
		{
			Comp->Deactivate();
		}
	}
}

void AVGMissionGimmickAltar::BeginPlay()
{
	Super::BeginPlay();
	
	for (int32 i = 0; i < PlacementSlots.Num(); i++)
	{
		const FVGAltarPlacementSlot& Slot = PlacementSlots[i];
        
		if (!Slot.RequiredItemHintEffect) continue;
        
		UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(this);
		NiagaraComp->SetAsset(Slot.RequiredItemHintEffect);
		NiagaraComp->SetupAttachment(RootComponent);
		NiagaraComp->SetRelativeLocation(Slot.AttachOffset);
		NiagaraComp->RegisterComponent();
        
		// 메쉬 정보 전달 (Niagara에 User.Mesh 파라미터가 있을 경우)
		UVGMissionItemDataAsset* ItemDataAsset = Slot.ItemDataAsset.Get();
		if (!ItemDataAsset)
		{
			continue;
		}
		
		NiagaraComp->SetVariableStaticMesh(FName("User.Mesh"), ItemDataAsset->ItemMesh);
        
		HintEffectComponents.Add(NiagaraComp);
	}
	
	// Player와의 거리 체크
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (LocalPC && LocalPC->IsLocalController())
	{
		GetWorldTimerManager().SetTimer(
			HintVisibilityTimerHandle,
			this,
			&AVGMissionGimmickAltar::UpdateHintEffectVisibility,
			0.3f,
			true
		);
	}
}

bool AVGMissionGimmickAltar::TryPlaceItemToSlot(UVGEquipmentComponent* EquipComp, FVGAltarPlacementSlot& Slot)
{
	// 헬퍼로 일치하는 아이템을 찾은 뒤 CarryItem으로 캐스팅
	UVGMissionItemDataAsset* ItemDataAsset = Slot.ItemDataAsset.Get();
	if (!ItemDataAsset)
	{
		return false;
	}
	
	AVGMissionItemBase* FoundItem =
		FindMissionItemByTag(EquipComp, ItemDataAsset->ItemTypeTag);
	
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

