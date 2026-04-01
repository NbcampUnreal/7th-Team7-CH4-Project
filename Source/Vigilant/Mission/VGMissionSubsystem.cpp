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
		Mission->OnMissionCompleted.AddDynamic(this, &UVGMissionSubsystem::OnMissionCompleted);
	}
}

void UVGMissionSubsystem::OnMissionCompleted(int32 MissionID)
{
	// 이미 완료된 미션 중복 처리 방지
	
	CompletedMissions.Add(MissionID);
}

TArray<AVGMissionBase*> UVGMissionSubsystem::GetAllMissions()
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
	
	return CompletedMissions / RegisteredMissions.Num();
}
