#include "Core/VGGameState.h"
#include "Core/VGPlayerState.h"
#include "Net/UnrealNetwork.h"

AVGGameState::AVGGameState()
{
	// 초기값 세팅
	CurrentPhaseTag = FGameplayTag::EmptyTag;
	TotalMissionProgress = 0.0f;
	LastDuelWinnerName = TEXT("");
	PhaseEndTime = 0.0f;
	PhaseStartTime = 0.0f;
	BossNerfRate = 1.0f;
	VotedPlayerName = TEXT("");
	VotedPlayerIndex = -1;
	bIsVoteTie = false;
}

void AVGGameState::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (CurrentPhaseTag.IsValid())
	{
		TagContainer.AddTag(CurrentPhaseTag);
	}
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

float AVGGameState::GetElapsedTime() const
{
	return FMath::Max(0.0f, GetServerWorldTimeSeconds() - PhaseStartTime);
}

void AVGGameState::SetCurrentPhaseTag(FGameplayTag NewTag)
{
	if (HasAuthority())
	{
		CurrentPhaseTag = NewTag;
		OnRep_CurrentPhaseTag(); 
	}
}

int32 AVGGameState::GetReadyPlayerCount() const
{
	int32 ReadyPlayerCount = 0;

	for (APlayerState* PlayerState : PlayerArray)
	{
		if (AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(PlayerState))
		{
			if (VGPlayerState->bIsReady)
			{
				ReadyPlayerCount++;
			}
		}
	}

	return ReadyPlayerCount;
}

int32 AVGGameState::GetTotalPlayerCount() const
{
	int32 TotalPlayerCount = 0;

	for (APlayerState* PlayerState : PlayerArray)
	{
		if (Cast<AVGPlayerState>(PlayerState))
		{
			TotalPlayerCount++;
		}
	}

	return TotalPlayerCount;
}
	

void AVGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AVGGameState, CurrentPhaseTag);
	DOREPLIFETIME(AVGGameState, TotalMissionProgress);
	DOREPLIFETIME(AVGGameState, LastDuelWinnerName);
	DOREPLIFETIME(AVGGameState, PhaseEndTime);
	DOREPLIFETIME(AVGGameState, PhaseStartTime);
	DOREPLIFETIME(AVGGameState, BossNerfRate);
	DOREPLIFETIME(AVGGameState, VotedPlayerName);
	DOREPLIFETIME(AVGGameState, VotedPlayerIndex);
	DOREPLIFETIME(AVGGameState, bIsVoteTie);
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

void AVGGameState::OnRep_PhaseStartTime()
{
	OnPhaseTimeChanged.Broadcast();
}

void AVGGameState::OnRep_PhaseEndTime(float OldEndTime)
{
	if (PhaseEndTime < OldEndTime)
	{
		// 미션 깨서 시간이 줄어든 경우
		UE_LOG(LogTemp, Warning, TEXT("[VGGameState] 시간이 단축되었습니다! (이전: %.1f -> 현재: %.1f)"), OldEndTime, PhaseEndTime);
	}
	else if (PhaseEndTime > OldEndTime)
	{
		// 막고라 다녀온 경우
		UE_LOG(LogTemp, Log, TEXT("[VGGameState] 페이즈 시간이 연장되었습니다. (이전: %.1f -> 현재: %.1f)"), OldEndTime, PhaseEndTime);
	}
	
	OnPhaseTimeChanged.Broadcast();
}

void AVGGameState::Multicast_PlayVoteResultCinematic_Implementation(int32 TargetEntryIndex)
{
	OnVoteResultCinematic.Broadcast(TargetEntryIndex);
}
