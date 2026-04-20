#include "VGMissionGimmickAltar.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Mission/Item/VGMissionItemCarry.h"
#include "Mission/Item/VGMissionItemBase.h"
#include "Data/VGMissionItemDataAsset.h"
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
	for (int32 Index = 0; Index < PlacementSlots.Num(); Index++)
	{
		const FVGAltarPlacementSlot& Slot = PlacementSlots[Index];
		if (IsSlotBitSet(Index))
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
		if (IsSlotBitSet(Index))
		{
			continue;
		}
		
		if (TryPlaceItemToSlot(EquipComp, Slot, Index))
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
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	APlayerController* LocalPC = World->GetFirstPlayerController();
	if (!IsValid(LocalPC))
	{
		return;
	}
	
	APawn* PlayerPawn = LocalPC->GetPawn();
	if (!IsValid(PlayerPawn))
	{
		return;
	}
	
	const float DistSq = FVector::DistSquared(
		GetActorLocation(),
		LocalPC->GetPawn()->GetActorLocation()
	);
    
	const bool bShouldShow = DistSq < FMath::Square(HintVisibleRange);
    
	// [Fix] 1:1 대응으로 구성되지만 런타임 안전을 위해 Min 사용
	const int32 Count = FMath::Min(HintEffectComponents.Num(), PlacementSlots.Num());
	for (int32 Index = 0; Index < Count; Index++)
	{
		UNiagaraComponent* Comp = HintEffectComponents[Index];
		if (!Comp)
		{
			continue;
		}
		
		// 이미 채워진 슬롯은 무조건 비활성
		const bool bSlotEmpty = !IsSlotBitSet(Index);
		const bool bActivate = bShouldShow && bSlotEmpty;
        
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
	PlacedItems.Init(nullptr, PlacementSlots.Num());
	
	for (int32 i = 0; i < PlacementSlots.Num(); i++)
	{
		const FVGAltarPlacementSlot& Slot = PlacementSlots[i];
        
		if (!Slot.RequiredItemHintEffect)
		{
			HintEffectComponents.Add(nullptr); // 인덱스 유지
			continue;
		}
        
		UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(this);
		NiagaraComp->SetAutoActivate(false);
		NiagaraComp->SetAsset(Slot.RequiredItemHintEffect);
		NiagaraComp->SetupAttachment(RootComponent);
		NiagaraComp->SetRelativeLocation(Slot.AttachOffset);
		NiagaraComp->RegisterComponent();
        
		// 메쉬 정보 전달 (Niagara에 User.Mesh 파라미터가 있을 경우)
		if (Slot.ItemDataAsset && Slot.ItemDataAsset->ItemMesh)
		{
			NiagaraComp->SetVariableObject(FName("User.TargetObject"), Slot.ItemDataAsset->ItemMesh);
		}
		
		NiagaraComp->ReinitializeSystem();
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

bool AVGMissionGimmickAltar::TryPlaceItemToSlot(UVGEquipmentComponent* EquipComp, FVGAltarPlacementSlot& Slot, int32 SlotIndex)
{
	// 헬퍼로 일치하는 아이템을 찾은 뒤 CarryItem으로 캐스팅
	if (!Slot.ItemDataAsset)
	{
		return false;
	}
	
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
	PlacedItems[SlotIndex] = CarryItem;
 
	UE_LOG(LogTemp, Warning, TEXT("[%s] Attach %s at %s"),
		*GetName(), *CarryItem->GetName(), *Slot.AttachOffset.ToString());
 
	return true;
}

bool AVGMissionGimmickAltar::AreAllSlotsFilled() const
{
	// PlacementSlots의 모든 슬롯 비트가 채워져 있는지 O(1) 비트 비교
	// PlacementSlots.Num()이 8을 넘으면 BeginPlay에서 ensure로 방어됨
	const uint8 FullMask = static_cast<uint8>((1 << PlacementSlots.Num()) - 1);
	return (PlacedSlotMask & FullMask) == FullMask;
}

