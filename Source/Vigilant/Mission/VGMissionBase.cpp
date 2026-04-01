#include "VGMissionBase.h"
#include "VGMissionGimmickBase.h"
#include "VGMissionItemBase.h"
#include "VGMissionSubsystem.h"
#include "Net/UnrealNetwork.h"

AVGMissionBase::AVGMissionBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// 초기 상태는 비활성
	CurrentStateTag = 
}

void AVGMissionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentStateTag);
}

bool AVGMissionBase::HasMissionTag(FGameplayTag Tag) const
{
	if (MissionTypeTag == Tag)
	{
		return true;
	}
	
	// MissionTags 컨테이너 포함 여부 확인
}

void AVGMissionBase::OnConditionMat()
{
}

void AVGMissionBase::OnRep_CurrentStateTag()
{
}

void AVGMissionBase::Server_CompleteMission()
{
}

void AVGMissionBase::NotifyMissionCompleted()
{
}


