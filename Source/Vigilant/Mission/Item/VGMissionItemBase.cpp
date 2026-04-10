#include "VGMissionItemBase.h"
#include "Mission/Definitions/VGMissionBase.h"
#include "Character/VGCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Common/VGGameplayTags.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Data/VGMissionItemDataAsset.h"

AVGMissionItemBase::AVGMissionItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	SetRootComponent(MeshComponent);
	MeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	ItemStateTag = VigilantMissionTags::ItemInactive;
}

void AVGMissionItemBase::SetStateTag(FGameplayTag NewStateTag)
{
	if (!HasAuthority())
	{
		return;
	}
	
	ItemStateTag = NewStateTag;
	OnRep_ItemStateTag();
	
	OnItemStateChanged.Broadcast(this, NewStateTag);
}

bool AVGMissionItemBase::CanInteractWith(AActor* Interactor) const
{
	// 이미 누군가 들고 있으면 상호작용 불가
	if (IsCarried()) return false;

	// 이미 사용됐거나 놓인 상태면 불가
	if (ItemStateTag != VigilantMissionTags::ItemInactive) return false;

	return Super::CanInteractWith(Interactor);
}

void AVGMissionItemBase::OnInteractWith(AActor* Interactor, const FTransform& InteractTransform)
{
	if (!HasAuthority())
	{
		return;
	}
	if (!CanInteractWith(Interactor))
	{
		return;
	}
	
	OnPickedUp(Interactor);
    
	if (UVGEquipmentComponent* EquipComp =
		Interactor->FindComponentByClass<UVGEquipmentComponent>())
	{
		EquipComp->Server_EquipItem(this);
		UE_LOG(LogTemp, Log, TEXT("[%s] Equipped!"),*GetName());
	}
}

void AVGMissionItemBase::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Carrier);
	DOREPLIFETIME(ThisClass, ItemStateTag);
}

// -----------------------------------------------
// 줍기 / 내려놓기 — 서버 전용
// -----------------------------------------------

void AVGMissionItemBase::OnPickedUp(AActor* NewCarrier)
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (AVGCharacterBase* CharacterCarrier = Cast<AVGCharacterBase>(NewCarrier))
	{
		Carrier = CharacterCarrier;
		
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
		SetStateTag(VigilantMissionTags::ItemCarried);
		// Carrier가 변경되었으므로 OnRep 수동 호출
		OnRep_Carrier();
	}
}

void AVGMissionItemBase::OnDropped()
{
	if (!HasAuthority())
	{
		return;
	}

	Carrier = nullptr;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SetStateTag(VigilantMissionTags::ItemInactive);
	OnRep_Carrier();
}

void AVGMissionItemBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (EquipmentData)
	{
		UVGMissionItemDataAsset* ItemDataAsset = Cast<UVGMissionItemDataAsset>(EquipmentData);
		
		if (ItemDataAsset)
		{
			UStaticMesh* Mesh = ItemDataAsset->ItemMesh;
			MeshComponent->SetStaticMesh(Mesh);
		
			ItemTypeTag = ItemDataAsset->ItemTypeTag;
		}
	}
}

// -----------------------------------------------
// 리플리케이션 콜백
// -----------------------------------------------

void AVGMissionItemBase::OnRep_Carrier()
{
	// TODO: 캐리 상태 변경에 따른 시각적 피드백 처리
	//       (예: 피킹 이펙트 재생, 아웃라인 제거 등)
	
	UE_LOG(LogTemp, Log, TEXT("[%s] Change carrier - %s"),*GetName(), *Carrier->GetName());
}

void AVGMissionItemBase::OnRep_ItemStateTag()
{
	// Todo State 변경에 따른 피드백 처리
	if (ItemStateTag == VigilantMissionTags::ItemCarried)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else if (ItemStateTag == VigilantMissionTags::ItemInactive)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}
