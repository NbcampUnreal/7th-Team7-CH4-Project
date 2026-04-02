#include "VGMissionGimmickBase.h"
#include "Net/UnrealNetwork.h"
#include "Mission/VGMissionBase.h"
#include "Common/VGGameplayTags.h"
#include "Misc/MapErrors.h"

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
	return Super::CanInteractWith(Interactor);
}

void AVGMissionGimmickBase::OnInteractWith(AVGCharacterBase* Interactor)
{
	Super::OnInteractWith(Interactor);
}

void AVGMissionGimmickBase::ReportConditionMet()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (!OwnerMission)
	{
		return;
	}
	
	// 중복 처리 방지
	if (GimmickStateTag == VigilantMissionTags::GimmickCompleted)
	{
		return;
	}
	
	// Todo Gimmick 달성 조건 체크
	SetGimmickState(VigilantMissionTags::GimmickCompleted);
	OwnerMission->OnConditionMet(this); 
}

void AVGMissionGimmickBase::SetGimmickState(FGameplayTag NewStateTag)
{
	if (!HasAuthority())
	{
		return;
	}
	
	GimmickStateTag = NewStateTag;
	OnRep_GimmickStateTag();
	
	OnGimmickStateChanged.Broadcast(this, NewStateTag);
}

void AVGMissionGimmickBase::OnRep_GimmickStateTag()
{
	// Todo State 변경에 따른 피드백 처리
}