#include "VGMissionGimmickAltar.h"
#include "Common/VGGameplayTags.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Mission/Item/VGMissionItemCarry.h"
#include "Mission/Item/VGMissionItemBase.h"
#include "Data/VGMissionItemDataAsset.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"

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
		
		if (!Slot.ItemDataAsset)
		{
			continue;
		}
		
		if (FindMissionItemByTag(EquipComp, Slot.ItemDataAsset->ItemTypeTag))
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
	for (int32 Index = 0; Index < PlacementSlots.Num(); Index++)
	{
		FVGAltarPlacementSlot& Slot = PlacementSlots[Index];
		if (Slot.IsOccupied())
		{
			continue;
		}
		
		if (TryPlaceItemToSlot(EquipComp, Slot))
		{
			SetSlotBit(Index);
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
    
	for (int32 Index = 0; Index < HintEffectComponents.Num(); Index++)
	{
		UNiagaraComponent* Comp = HintEffectComponents[Index];
		if (!Comp)
		{
			continue;
		}
		
		// 이미 채워진 슬롯은 무조건 비활성
		bool bSlotEmpty = !IsSlotBitSet(Index);
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
	
	// PlacedSlotMask(uint8) 한계 체크 — 8슬롯 초과 시 비트 오버플로 발생
	ensureMsgf(PlacementSlots.Num() <= 8,
		TEXT("[%s] PlacementSlots.Num() = %d exceeds PlacedSlotMask(uint8) limit(8)."),
		*GetName(), PlacementSlots.Num());
	
	// PlacementSlots와 1:1 인덱스 매칭 유지 (힌트가 없는 슬롯도 nullptr로 채움)
	HintEffectComponents.Reserve(PlacementSlots.Num());
	for (const FVGAltarPlacementSlot& Slot : PlacementSlots)
	{
		if (!Slot.RequiredItemHintEffect)
		{
			HintEffectComponents.Add(nullptr);
			continue;
		}
		
		UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(this);
		NiagaraComp->SetAutoActivate(false);
		NiagaraComp->SetAsset(Slot.RequiredItemHintEffect);
		NiagaraComp->SetupAttachment(RootComponent);
		NiagaraComp->SetRelativeLocation(Slot.AttachOffset);
		NiagaraComp->RegisterComponent();
		
		// Niagara의 User.TargetObject 파라미터에 메쉬를 전달 (Static Mesh Location 모듈에서 오브젝트 타입으로만 바인딩 가능)
		if (Slot.ItemDataAsset && Slot.ItemDataAsset->ItemMesh)
		{
			NiagaraComp->SetVariableObject(FName("User.TargetObject"), Slot.ItemDataAsset->ItemMesh);
		}
		
		NiagaraComp->ReinitializeSystem();
		HintEffectComponents.Add(NiagaraComp);
	}
	
	// 거리 기반 표시 갱신은 로컬 플레이어에서만 수행
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

void AVGMissionGimmickAltar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlacedSlotMask);
}

void AVGMissionGimmickAltar::SetSlotBit(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= PlacementSlots.Num())
	{
		return;
	}
	
	PlacedSlotMask |= (1 << SlotIndex);
}

bool AVGMissionGimmickAltar::IsSlotBitSet(int32 SlotIndex) const
{
	return PlacedSlotMask & (1 << SlotIndex);
}

bool AVGMissionGimmickAltar::TryPlaceItemToSlot(UVGEquipmentComponent* EquipComp, FVGAltarPlacementSlot& Slot)
{
	if (!Slot.ItemDataAsset)
	{
		return false;
	}
	
	// 일치하는 아이템을 찾은 뒤 CarryItem으로 캐스팅
	AVGMissionItemBase* FoundItem =
		FindMissionItemByTag(EquipComp, Slot.ItemDataAsset->ItemTypeTag);
	
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
	
	return true;
}

bool AVGMissionGimmickAltar::AreAllSlotsFilled() const
{
	// PlacementSlots의 모든 슬롯 비트가 채워져 있는지 O(1) 비트 비교
	// PlacementSlots.Num()이 8을 넘으면 BeginPlay에서 ensure로 방어됨
	const uint8 FullMask = static_cast<uint8>((1 << PlacementSlots.Num()) - 1);
	return (PlacedSlotMask & FullMask) == FullMask;
}

