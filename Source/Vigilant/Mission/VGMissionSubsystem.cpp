#include "VGMissionSubsystem.h"
#include "VGMissionBase.h"

void UVGMissionSubsystem::RegisterMission(AVGMissionBase* Mission)
{
	if (!Mission)
	{
		return;
	}
	
	if (RegisteredMissions.Contains(Mission) == false)
	{
		RegisteredMissions.AddUnique(Mission);
		// 완료 이벤트 - 진행도 계산용
		Mission->OnMissionCompleted.AddDynamic(this, &UVGMissionSubsystem::OnMissionCompleted);
		// 상태 전환 이벤트 - 맵 표시 / HUD 갱신용
		// Subsystem의 OnMissionStateChanged으로 MissionBase의 델리게이트를 구독
		Mission->OnMissionStateChanged.AddDynamic(this, &UVGMissionSubsystem::OnMissionStateChanged);
	}
}

void UVGMissionSubsystem::OnMissionCompleted(int32 MissionID)
{
	// 이미 완료된 미션 중복 처리 방지
	if (CompletedMissions.Contains(MissionID))
	{
		return;
	}
	
	CompletedMissions.Add(MissionID);
	
	// 전체 완료 시
	if (CompletedMissions.Num() == RegisteredMissions.Num())
	{
		OnAllMissionCompleted.Broadcast();
	}
}

TArray<AVGMissionBase*> UVGMissionSubsystem::GetMissionsByTag(FGameplayTag TypeTag) const
{
	TArray<AVGMissionBase*> Missions;
	for (AVGMissionBase* Mission : RegisteredMissions)
	{
		if (Mission)
		{
			if(Mission->HasMissionTag(TypeTag) || Mission->GetCurrentStateTag() == TypeTag)
			{
				Missions.Add(Mission);
			}
		}
	}
	
	return Missions;
}

AVGMissionBase* UVGMissionSubsystem::GetMissionsByID(int32 MissionID) const
{
	for (AVGMissionBase* Mission : RegisteredMissions)
	{
		if (Mission->GetMissionID() == MissionID)
		{
			return Mission;
		}
	}
	
	return nullptr;
}

TArray<AVGMissionBase*> UVGMissionSubsystem::GetAllMissions() const
{
	return RegisteredMissions;
}

int32 UVGMissionSubsystem::GetMissionCountByState(FGameplayTag StateTag) const
{
	int32 Count = 0;
	for (const AVGMissionBase* Mission : RegisteredMissions)
	{
		if (Mission->HasMissionTag(StateTag))
		{
			Count++;
		}
	}
	
	return Count;
}

float UVGMissionSubsystem::GetMissionProgress() const
{
	if (RegisteredMissions.IsEmpty())
	{
		return 0.f;
	}
	
	return static_cast<float>(CompletedMissions.Num()) / RegisteredMissions.Num();
}
