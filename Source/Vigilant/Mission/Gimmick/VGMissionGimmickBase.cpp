#include "VGMissionGimmickBase.h"
#include "Net/UnrealNetwork.h"
#include "Mission/VGMissionBase.h"

AVGMissionGimmickBase::AVGMissionGimmickBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
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
	
	// Todo Gimmick 달성 조건 체크
	SetGimmickState(FGameplayTag::RequestGameplayTag(FName("Mission.State.Completed")));
	OwnerMission->OnConditionMet(); 
}

void AVGMissionGimmickBase::SetGimmickState(FGameplayTag NewStateTag)
{
	if (!HasAuthority())
	{
		return;
	}
	
	GimmickStateTag = NewStateTag;
	OnRep_GimmickStateTag();
}

void AVGMissionGimmickBase::OnRep_GimmickStateTag()
{
	// Todo State 변경에 따른 피드백 처리
}