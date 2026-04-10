#include "VGMissionSubsystem.h"
#include "Mission/Definitions/VGMissionBase.h"
#include "Common/VGGameplayTags.h"

void UVGMissionSubsystem::Server_RegisterMission(AVGMissionBase* Mission)
{
	if (!Mission)
	{
		return;
	}
	// [Fix] MissionID 유효성 검증 — 기본값(-1)이나 중복 ID는 완료 판정 오류를 유발함
	const int32 ID = Mission->GetMissionID();
	if (ID < 0)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Subsystem] Mission '%s' has invalid MissionID(%d). 에디터에서 MissionID를 설정하세요."),
			*Mission->GetName(), ID);
		return;
	}
	
	for (const AVGMissionBase* Existing : RegisteredMissions)
	{
		if (Existing && Existing->GetMissionID() == ID)
		{
			UE_LOG(LogTemp, Error,
				TEXT("[Subsystem] MissionID(%d) 중복! '%s'와 '%s'. 고유 ID를 부여하세요."),
				ID, *Existing->GetName(), *Mission->GetName());
			return;
		}
	}
	
	if (RegisteredMissions.Contains(Mission) == false)
	{
		RegisteredMissions.AddUnique(Mission);
		// 완료 이벤트 - 진행도 계산용
		Mission->OnMissionCompleted.AddDynamic(this, &UVGMissionSubsystem::Server_OnMissionCompleted);
		
		// 상태 전환 이벤트 - 맵 표시 / HUD 갱신용
		Mission->OnMissionStateChanged.AddDynamic(this, &UVGMissionSubsystem::HandleMissionStateChanged);
	}
	UE_LOG(LogTemp, Warning,
		TEXT("[Subsystem] Registered Mission: %s (ID=%d)"),
		*Mission->GetName(), Mission->GetMissionID());
}

void UVGMissionSubsystem::Client_RegisterMission(AVGMissionBase* Mission)
{
	if (IsServer())
	{
		return;
	}
	
	if (!Mission)
	{
		return;
	}
	
	if (RegisteredMissions.Contains(Mission) == false)
	{
		RegisteredMissions.AddUnique(Mission);
		Mission->OnMissionStateChanged.AddDynamic(
			this, &UVGMissionSubsystem::HandleMissionStateChanged);
		
		OnMissionRegistered.Broadcast(Mission); // UI 갱신 트리거
	}
}

void UVGMissionSubsystem::Server_OnMissionCompleted_Implementation(int32 MissionID)
{
	AVGMissionBase* Mission = GetMissionByID(MissionID);
	if (Mission)
	{
		float ReduceTime = Mission->GetMissionClearReduceTime();
		OnMissionClearTimeReward.Broadcast(ReduceTime);
	}
	
	// 전체 완료 체크
	int32 CompletedCount = GetMissionCountByState(VigilantMissionTags::MissionCompleted);
	if (CompletedCount == RegisteredMissions.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] All Mission Clear!"));
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
			// [Fix] HasMissionTag 내부에서 이미 GetCurrentStateTag() 비교를 수행하므로 중복 조건 제거
			if(Mission->HasMissionTag(TypeTag))
			{
				Missions.Add(Mission);
			}
		}
	}
	
	return Missions;
}

AVGMissionBase* UVGMissionSubsystem::GetMissionByID(int32 MissionID) const
{
	for (AVGMissionBase* Mission : RegisteredMissions)
	{
		if (Mission && Mission->GetMissionID() == MissionID)
		{
			return Mission;
		}
	}
	
	return nullptr;
}

const TArray<AVGMissionBase*>& UVGMissionSubsystem::GetAllMissions() const
{
	return RegisteredMissions;
}

int32 UVGMissionSubsystem::GetMissionCountByState(FGameplayTag StateTag) const
{
	int32 Count = 0;
	for (const AVGMissionBase* Mission : RegisteredMissions)
	{
		// [Fix] HasMissionTag는 타입 태그까지 매칭하므로, 상태만 비교하도록 수정
		if (Mission && Mission->GetCurrentStateTag() == StateTag)
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
	
	int32 CompletedCount = GetMissionCountByState(VigilantMissionTags::MissionCompleted);
	return static_cast<float>(CompletedCount) / RegisteredMissions.Num();
}

void UVGMissionSubsystem::HandleMissionStateChanged(int32 MissionID, FGameplayTag NewStateTag)
{
	OnMissionStateChanged.Broadcast(MissionID, NewStateTag);
}

bool UVGMissionSubsystem::IsServer() const
{
	return GetWorld() && GetWorld()->GetNetMode() != NM_Client;
}