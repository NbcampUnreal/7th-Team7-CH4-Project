#include "VGMissionGimmickAltar.h"
#include "Common/VGGameplayTags.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Mission/Item/VGMissionItemCarry.h"
#include "Mission/Item/VGMissionItemBase.h"
#include "Data/VGMissionItemDataAsset.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

AVGMissionGimmickAltar::AVGMissionGimmickAltar()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GimmickTypeTag = VigilantMissionTags::AltarGimmick;
}

bool AVGMissionGimmickAltar::CanInteractWith(AActor* Interactor) const
{
	if (GimmickStateTag != VigilantMissionTags::GimmickInactive)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[%s] Cannot interact: state = %s"), *GetName(), *GimmickStateTag.ToString());
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
	
	const float DistSq = FVector::DistSquared(
		GetActorLocation(),
		LocalPC->GetPawn()->GetActorLocation()
	);
	const bool bShouldShow = DistSq < FMath::Square(HintVisibleRange);
	
	// BeginPlay에서 1:1 대응으로 구성되지만 런타임 안전을 위해 Min 사용
	const int32 Count = FMath::Min(HintEffectComponents.Num(), PlacementSlots.Num());
	for (int32 Index = 0; Index < Count; Index++)
	{
		UNiagaraComponent* Comp = HintEffectComponents[Index];
		if (!Comp)
		{
			continue;
		}
		
		// 이미 채워진 슬롯은 무조건 비활성
		const bool bSlotEmpty = !PlacementSlots[Index].IsOccupied();
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
	
	// PlacementSlots와 1:1 인덱스 매칭 유지 (힌트 이펙트가 없는 슬롯도 nullptr로 채움)
	HintEffectComponents.Reserve(PlacementSlots.Num());
	for (const FVGAltarPlacementSlot& Slot : PlacementSlots)
	{
		if (!Slot.RequiredItemHintEffect)
		{
			HintEffectComponents.Add(nullptr);
			continue;
		}
		
		UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(this);
		NiagaraComp->SetAsset(Slot.RequiredItemHintEffect);
		NiagaraComp->SetupAttachment(RootComponent);
		NiagaraComp->SetRelativeLocation(Slot.AttachOffset);
		NiagaraComp->SetAutoActivate(false);
		NiagaraComp->RegisterComponent();
		
		// Niagara에 User.Mesh 파라미터가 있을 경우 메쉬 정보 전달
		if (Slot.ItemDataAsset && Slot.ItemDataAsset->ItemMesh)
		{
			NiagaraComp->SetVariableStaticMesh(FName("User.Mesh"), Slot.ItemDataAsset->ItemMesh);
		}
		
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
	for (const FVGAltarPlacementSlot& Slot : PlacementSlots)
	{
		if (!Slot.IsOccupied())
		{
			return false;
		}
	}
	return true;
}
