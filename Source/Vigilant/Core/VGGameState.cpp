#include "Core/VGGameState.h"
#include "Net/UnrealNetwork.h"

AVGGameState::AVGGameState()
{
	// 초기값 세팅
	CurrentPhaseTag = FGameplayTag::EmptyTag;
	TotalMissionProgress = 0.0f;
	LastDuelWinnerName = TEXT("");
	PhaseEndTime = 0.0f;
	BossNerfRate = 1.0f;
}

float AVGGameState::GetRemainingPhaseTime() const
{
	if (PhaseEndTime < 0.0f)
	{
		return -1.0f;
	}
	float TimeLeft = PhaseEndTime - GetServerWorldTimeSeconds();
	
	return FMath::Max(0.0f, TimeLeft);
}

void AVGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AVGGameState, CurrentPhaseTag);
	DOREPLIFETIME(AVGGameState, TotalMissionProgress);
	DOREPLIFETIME(AVGGameState, LastDuelWinnerName);
	DOREPLIFETIME(AVGGameState, PhaseEndTime);
	DOREPLIFETIME(AVGGameState, BossNerfRate);
}

void AVGGameState::OnRep_CurrentPhaseTag()
{
	OnPhaseChanged.Broadcast(CurrentPhaseTag);
}

void AVGGameState::OnRep_MissionProgress()
{
	OnMissionProgressUpdated.Broadcast(TotalMissionProgress);
}

void AVGGameState::OnRep_DuelWinner()
{
	if (!LastDuelWinnerName.IsEmpty())
	{
		OnDuelWinnerAnnounced.Broadcast(LastDuelWinnerName);
	}
}

void AVGGameState::OnRep_BossNerfRate()
{
	OnBossNerfUpdated.Broadcast(BossNerfRate);
}
