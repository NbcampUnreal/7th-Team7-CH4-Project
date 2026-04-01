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
	if (CompletedMissions.Contains(MissionID))
	{
		return;
	}
	
	CompletedMissions.Add(MissionID);
	
	if (AVGMissionBase* Mission = GetMissionsByID(MissionID))
	{
		OnMissionStateChanged.Broadcast(MissionID, Mission->GetCurrentStateTag());
	}
	
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
		if (Mission && Mission->HasMissionTag(TypeTag))
		{
			Missions.Add(Mission);
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
	
	return CompletedMissions.Num() / RegisteredMissions.Num();
}
