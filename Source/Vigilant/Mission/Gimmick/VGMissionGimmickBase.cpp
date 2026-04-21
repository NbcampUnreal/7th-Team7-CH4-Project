#include "VGMissionGimmickBase.h"
#include "Net/UnrealNetwork.h"
#include "Common/VGGameplayTags.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Character/VGCharacterBase.h"
#include "Mission/Item/VGMissionItemBase.h"
#include "Data/VGMissionItemDataAsset.h"
#include "Components/SceneComponent.h"

AVGMissionGimmickBase::AVGMissionGimmickBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	RootComp = CreateDefaultSubobject<USceneComponent>("RootComp");
	SetRootComponent(RootComp);
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	MeshComponent->SetupAttachment(RootComp);
	MeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	
	GimmickStateTag = VigilantMissionTags::GimmickInactive;
	
	Tags.Add(FName("InteractTarget"));
}

void AVGMissionGimmickBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 동적 머티리얼 생성
	if (!BodyDynMat)
	{
		// [Fix] 메시에 머티리얼이 없을 경우 GetMaterial(0)이 nullptr → Create 크래시 방지
		UMaterialInterface* BaseMaterial = MeshComponent ? MeshComponent->GetMaterial(0) : nullptr;
		if (!BaseMaterial)
		{
			return;
		}
		BodyDynMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		if (!BodyDynMat)
		{
			return;
		}
		
		MeshComponent->SetMaterial(0, BodyDynMat);
	}
}

void AVGMissionGimmickBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, GimmickStateTag);
}

bool AVGMissionGimmickBase::CanInteractWith(AActor* Interactor) const
{
	return false;
}

void AVGMissionGimmickBase::OnInteractWith(AActor* Interactor, const FTransform& InteractTransform)
{
	if (!CanInteractWith(Interactor))
	{
		return;
	}
	
	OnGimmickInteracted.Broadcast(this, Interactor);
}

void AVGMissionGimmickBase::ResetGimmickState()
{
	if (!HasAuthority()) return;
	if (GimmickStateTag == VigilantMissionTags::GimmickCompleted) return;

	SetStateTag(VigilantMissionTags::GimmickInactive);
}

void AVGMissionGimmickBase::SetGimmickIndex(int32 NewGimmickIndex)
{
	GimmickIndex = NewGimmickIndex;
}

void AVGMissionGimmickBase::SetStateTag(FGameplayTag NewStateTag)
{
	if (!HasAuthority())
	{
		return;
	}
	
	// Completed 이후 상태 변경 차단
	if (GimmickStateTag == VigilantMissionTags::GimmickCompleted)
	{
		return;
	}
	
	// 잘못된 태그가 들어오는 것을 방지
	ensureMsgf(
		NewStateTag == VigilantMissionTags::GimmickInactive ||
		NewStateTag == VigilantMissionTags::GimmickActive   ||
		NewStateTag == VigilantMissionTags::GimmickCompleted,
		TEXT("[%s] SetStateTag — Invalid tag for Gimmick: %s"),
		*GetName(), *NewStateTag.ToString());
	
	GimmickStateTag = NewStateTag;
	OnRep_GimmickStateTag();
	
	OnGimmickStateChanged.Broadcast(this, NewStateTag);
}

AVGMissionItemBase* AVGMissionGimmickBase::FindMissionItemByTag(UVGEquipmentComponent* EquipComp,
	FGameplayTag RequiredTag) const
{
	if (!EquipComp)
	{
		return nullptr;
	}
 
	for (AVGEquippableActor* HandItem : { EquipComp->LeftHandItem, EquipComp->RightHandItem })
	{
		if (!HandItem || !HandItem->EquipmentData)
		{
			continue;
		}
 
		UVGMissionItemDataAsset* ItemData =
			Cast<UVGMissionItemDataAsset>(HandItem->EquipmentData);
 
		if (ItemData && ItemData->ItemTypeTag == RequiredTag)
		{
			return Cast<AVGMissionItemBase>(HandItem);
		}
	}
 
	return nullptr;
}

void AVGMissionGimmickBase::OnRep_GimmickStateTag()
{
	// BP 상에서의 로직 실행
	BP_OnGimmickStateTagChanged(GetStateTag());
	
	// State에 따른 색변경 처리
	FLinearColor Color = InactiveColor;
	FLinearColor EmissiveColor = InactiveEmissiveColor;
	
	if (GimmickStateTag == VigilantMissionTags::GimmickCompleted)
	{
		Color = CompleteColor;
		EmissiveColor = CompleteEmissiveColor;
		
		MeshComponent->SetOverlayMaterial(nullptr);
	}
	else if (GimmickStateTag == VigilantMissionTags::GimmickActive)
	{
		Color = ActiveColor;
		EmissiveColor = ActiveEmissiveColor;
	}
	
	BodyDynMat->SetVectorParameterValue(TEXT("Color"), Color);
	BodyDynMat->SetVectorParameterValue(TEXT("EmissiveColor"), EmissiveColor);
}
