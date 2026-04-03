#include "VGMissionBase.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"
#include "Mission/Item/VGMissionItemBase.h"
#include "Mission/VGMissionSubsystem.h"
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

void AVGMissionBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		if (UVGMissionSubsystem* Subsystem =
			GetWorld()->GetSubsystem<UVGMissionSubsystem>())
		{
			Subsystem->RegisterMission(this);
		}
		
		// 에디터에서 등록된 기믹들에 바인딩
		for (int32 i = 0; i < MissionGimmicks.Num(); i++)
		{
			AVGMissionGimmickBase* Gimmick = MissionGimmicks[i];
			if (Gimmick)
			{
				Gimmick->SetGimmickIndex(i); // 자동 인덱스 부여
				Gimmick->OnGimmickStateChanged.AddDynamic(
					this, &AVGMissionBase::OnGimmickStateChanged);
			}
		}
		
		// 에디터에서 등록된 Item 바인딩
		for (AVGMissionItemBase* Item : MissionItems)
		{
			if (Item)
			{
				Item->OnItemStateChanged.AddDynamic(
					this, &AVGMissionBase::OnItemStateChanged);
			}
		}
	}
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
	
	// 완료 상태를 외부에 전달
	if (CurrentStateTag == VigilantMissionTags::MissionCompleted)
	{
		NotifyMissionCompleted();
	}
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

void AVGMissionBase::OnRep_CurrentStateTag()
{
	// Todo State 변경에 따른 피드백 처리
	if (CurrentStateTag == VigilantMissionTags::MissionCompleted)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Mission Clear!"), *GetName());
		
		if (HasAuthority())
		{
			for (AVGMissionGimmickBase* Gimmick : MissionGimmicks)
			{
				if (Gimmick)
				{
					Gimmick->SetStateTag(VigilantMissionTags::GimmickCompleted);
				}
			}
		
			for (AVGMissionItemBase* Item : MissionItems)
			{
				if (Item)
				{
					// Item->SetStateTag(VigilantMissionTags::MissionCompleted);
				}
			}
		}
	}
}

void AVGMissionBase::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
}

void AVGMissionBase::OnItemStateChanged(AVGMissionItemBase* Gimmick, FGameplayTag Tag)
{
}

bool AVGMissionBase::CheckMissionCondition(AActor* Reporter)
{
	// 자식이 override 안 하면 런타임에 경고
	ensureMsgf(false, 
		TEXT("CheckMissionCondition must be overridden in %s"),
		*GetClass()->GetName());
	return false;
}

void AVGMissionBase::CompleteMission()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (CurrentStateTag == VigilantMissionTags::MissionCompleted)
	{
		return;
	}
    
	SetMissionState(VigilantMissionTags::MissionCompleted);
}

void AVGMissionBase::NotifyMissionCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[%s] Mission Completed!"), *GetName());
	// UI 및 외부 시스템용 델리게이트 브로드캐스트
	OnMissionCompleted.Broadcast(MissionID);
}
