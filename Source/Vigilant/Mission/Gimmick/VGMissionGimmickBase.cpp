#include "VGMissionGimmickBase.h"
#include "Net/UnrealNetwork.h"
#include "Common/VGGameplayTags.h"

AVGMissionGimmickBase::AVGMissionGimmickBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	SetRootComponent(MeshComponent);
}

void AVGMissionGimmickBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, GimmickStateTag);
}

bool AVGMissionGimmickBase::CanInteractWith(AVGCharacterBase* Interactor) const
{
	return false;
}

void AVGMissionGimmickBase::OnInteractWith(AVGCharacterBase* Interactor)
{
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

void AVGMissionGimmickBase::OnRep_GimmickStateTag()
{
	// Todo State 변경에 따른 피드백 처리
	if (!DynamicMaterialInstance)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MeshComponent->GetMaterial(0), this);
		MeshComponent->SetMaterial(0, DynamicMaterialInstance);
	}
	
	if (!DynamicMaterialInstance)
	{
		return;
	}
	
	FLinearColor Color = FLinearColor::White;
	if (GimmickStateTag == VigilantMissionTags::GimmickCompleted)
	{
		Color = FColor::Black;
	}
	else if (GimmickStateTag == VigilantMissionTags::GimmickActive)
	{
		Color = FColor::Cyan;
	}
	
	DynamicMaterialInstance->SetVectorParameterValue(TEXT("Color"), Color);
}