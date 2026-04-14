#include "VGVotePhase.h"
#include "Core/VGGameMode.h"
#include "Core/VGPlayerState.h"
#include "Common/VGGameplayTags.h"
#include "TimerManager.h"

void UVGVotePhase::EnterPhase()
{
	Super::EnterPhase();
	
	PlayerVotes.Empty();
	
	if (GameModeRef)
	{
		GameModeRef->GetWorldTimerManager().SetTimer(
			PhaseTimerHandle, 
			this, 
			&UVGVotePhase::OnVoteTimeUp, 
			PhaseDuration, 
			false);
	}
}

void UVGVotePhase::ExitPhase()
{
	if (GameModeRef)
	{
		GameModeRef->GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	}
	
	Super::ExitPhase();
}

void UVGVotePhase::ExecutePhaseResult()
{
	if (GameModeRef)
	{
		GameModeRef->NotifyPhaseCompleted(this);
	}
}

bool UVGVotePhase::CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target)
{
	return false;
}

bool UVGVotePhase::CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target)
{
	return false;
}
void UVGVotePhase::OnVoteTimeUp()
{
	CalculateVoteResult();
	
	ExecutePhaseResult();
}

void UVGVotePhase::ProcessVote(AVGPlayerState* Voter, AVGPlayerState* VotedTarget)
{
	if (!Voter || !VotedTarget) return;
	
	// 투표한 사람이 다시 투표하면 투표못하게 반환
	if (PlayerVotes.Contains(Voter))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VGVotePhase] %s 플레이어는 이미 투표했습니다."), *Voter->GetPlayerName());
		return;
	}
	
	PlayerVotes.Add(Voter, VotedTarget);
	
	int32 PlayerCount = 0;
	if (GameModeRef && GameModeRef->GameState)
	{
		for (APlayerState* PlayerState : GameModeRef->GameState->PlayerArray)
		{
			AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(PlayerState);
			if (VGPlayerState) 
			{
				PlayerCount++;
			}
		}
	}

	// 모든 인원이 투표 완료했다면 즉시 결과 실행
	if (PlayerVotes.Num() >= PlayerCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VGVotePhase] 모든 인원 투표 완료. 페이즈를 종료합니다."));
		OnVoteTimeUp();
	}
}

void UVGVotePhase::CalculateVoteResult()
{
	// 전부 투표 안했으면 동률로 처리 수정 예정(MVP 기획 상으로는 동률일 때 아무 효과가 없으므로 이대로 두어도 무방)
	if (PlayerVotes.Num() == 0)
	{
		return;
	}
	
	// 투표 계산할 임시 맵
	TMap<AVGPlayerState*, int32> VoteCounts;
	
	for (const auto& Pair : PlayerVotes)
	{
		AVGPlayerState* Target = Pair.Value;
		VoteCounts.FindOrAdd(Target) += 1;
	}
	
	AVGPlayerState* MaxVotedPlayer = nullptr;
	int32 MaxVotes = 0;
	bool bIsTie = false;
	
	for (const auto& Pair : VoteCounts)
	{
		if (Pair.Value > MaxVotes)
		{
			MaxVotes = Pair.Value;
			MaxVotedPlayer = Pair.Key;
			bIsTie = false;
		}
		else if (Pair.Value == MaxVotes)
		{
			bIsTie = true;
		}
	}
	
	if (MaxVotedPlayer && !bIsTie)
	{
		if (AVGGameState* VGGameState = GameModeRef->GetWorld()->GetGameState<AVGGameState>())
		{
			// 마피아 태그가 있는지 확인
			if (MaxVotedPlayer->IsRole(VigilantRoleTags::Mafia))
			{
				VGGameState->BossNerfRate -= BossStatChangeAmount;
				UE_LOG(LogTemp, Warning, TEXT("[VGVotePhase] 마피아 검거! "));
			}
			// 시민 태그가 있는지 확인
			else if (MaxVotedPlayer->IsRole(VigilantRoleTags::Citizen))
			{
				VGGameState->BossNerfRate += BossStatChangeAmount;
				UE_LOG(LogTemp, Warning, TEXT("[VGVotePhase] 시민 검거! "));
			}

			// 값이 비정상적으로 튀지 않도록 안전장치
			VGGameState->BossNerfRate = FMath::Clamp(VGGameState->BossNerfRate, 0.1f, 2.0f);

			// 클라이언트 UI 갱신을 위해 수동 방송(X)
			VGGameState->OnBossNerfUpdated.Broadcast(VGGameState->BossNerfRate);
		}
	}
	else if (bIsTie)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VGVotePhase] 동표 발생. 보스 스탯에 변동이 없습니다."));
	}
}
