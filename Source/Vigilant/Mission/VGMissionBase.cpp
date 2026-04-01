#include "VGMissionBase.h"
#include "Gimmick/VGMissionGimmickBase.h"
#include "Item/VGMissionItemBase.h"
#include "VGMissionSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Common/VGGameplayTags.h"

AVGMissionBase::AVGMissionBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// 초기 상태는 비활성
	CurrentStateTag = VigilantMissionTags::MissionInactive;
}

void AVGMissionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentStateTag);
}

void AVGMissionBase::SetMissionState(FGameplayTag NewStateTag)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentStateTag = NewStateTag;

	// 서버는 OnRep가 자동 호출되지 않으므로 직접 호출
	OnRep_CurrentStateTag();
	
	// 모든 상태 전환을 외부에 전달
    OnMissionStateChanged.Broadcast(MissionID, NewStateTag);
}

bool AVGMissionBase::HasMissionTag(FGameplayTag Tag) const
{
	if (MissionTypeTag == Tag)
	{
		return true;
	}
	else if (GetCurrentStateTag() == Tag)
	{
		return true;
	}
	
	// MissionTags 컨테이너 포함 여부 확인
	return MissionTags.HasTag(Tag);
}

int32 AVGMissionBase::GetMissionID() const
{
	return MissionID;
}

void AVGMissionBase::OnConditionMet()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (CurrentStateTag == VigilantMissionTags::MissionCompleted)
	{
		return;
	}
	
	// Todo 미션별 달성 조건 판단 로직 구현
	CompleteMission();
}

void AVGMissionBase::OnRep_CurrentStateTag()
{
	// Todo State 변경에 따른 피드백 처리
}

void AVGMissionBase::CompleteMission()
{
	if (!HasAuthority())
	{
		return;
	}
	
    SetMissionState(VigilantMissionTags::MissionCompleted);
	NotifyMissionCompleted();
}

void AVGMissionBase::NotifyMissionCompleted()
{
	// Subsystem에 완료 보고
	if (UVGMissionSubsystem* MissionSubsystem =
		GetWorld()->GetSubsystem<UVGMissionSubsystem>())
	{
		MissionSubsystem->OnMissionCompleted(MissionID);
	}

	// UI 및 외부 시스템용 델리게이트 브로드캐스트
	OnMissionCompleted.Broadcast(MissionID);
}