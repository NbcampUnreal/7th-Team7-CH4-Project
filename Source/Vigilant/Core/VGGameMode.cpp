#include "Core/VGGameMode.h"
#include "gameframework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Core/GamePhases/VGPhaseBase.h"
#include "Core/VGPlayerState.h"
#include "Core/VGGameState.h"
#include "Algo/RandomShuffle.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"


void AVGGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 게임 시작 페이즈 세팅"));
	
	if (InitialPhase)
	{
		PushPhase(InitialPhase); 
	}
	
}

void AVGGameMode::ClearDuelParticipants()
{
	DuelChallenger = nullptr;
	DuelTarget = nullptr;
}

void AVGGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (AVGPlayerState* VGPlayerState = NewPlayer->GetPlayerState<AVGPlayerState>())
	{
		// 입장 순서 부여
		ConnectedPlayerCount++;
		VGPlayerState->EntryIndex = ConnectedPlayerCount;
		
		// 해당 인덱스에 맞는 감옥으로 강제 텔레포트!
		if (AVGCharacterBase* VGPlayerCharcter = Cast<AVGCharacterBase>(NewPlayer->GetPawn()))
		{
			// 각 스폰포인트(감옥)으로 텔레포트하는 로직 구현 예정
		}

		UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] %d번 플레이어 배정 완료"), ConnectedPlayerCount);
	}
}

void AVGGameMode::TransitionToPhase(TSubclassOf<class UVGPhaseBase> NextPhase)
{
	PopPhase();
	
	if (NextPhase != nullptr)
	{
		PushPhase(NextPhase);
	}
}

void AVGGameMode::PushPhase(TSubclassOf<class UVGPhaseBase> NewPhase)
{
	if (NewPhase == nullptr) return;
	
	if (PhaseStack.Num() > 0)
	{
		PhaseStack.Last()->PausePhase();
	}
	
	UVGPhaseBase* CreatedPhase = NewObject<UVGPhaseBase>(this, NewPhase);
	if (CreatedPhase)
	{
		CreatedPhase->InitializePhase(this);
		PhaseStack.Push(CreatedPhase);
		
		if (AVGGameState* VGGameState = GetGameState<AVGGameState>())
		{
			VGGameState->CurrentPhaseTag = CreatedPhase->PhaseTag; 
			if (CreatedPhase->bHasTimeLimit)
			{
				VGGameState->PhaseEndTime = GetWorld()->GetTimeSeconds() + CreatedPhase->PhaseDuration;
			}
			else
			{
				VGGameState->PhaseEndTime = -1.0f;
			}
		}
		
		CreatedPhase->EnterPhase();
	}
}

void AVGGameMode::PopPhase()
{
	// 가장 최근에 실행된 페이즈 있으면 종료 후 스택에서 제거
	if (PhaseStack.Num() > 0)
	{
		UVGPhaseBase* EndingPhase = PhaseStack.Pop();
		EndingPhase->ExitPhase();
	}
	
	// 제거 후 스택에 남아있는 페이즈 있으면 재시작
	if (PhaseStack.Num() > 0)
	{
		UVGPhaseBase* ResumedPhase = PhaseStack.Last();
		PhaseStack.Last()->ResumePhase();
		
		if (AVGGameState* VGGameState = GetGameState<AVGGameState>())
		{
			VGGameState->CurrentPhaseTag = ResumedPhase->PhaseTag;
		}
	}
}



void AVGGameMode::CheckWinCondition()
{
}

void AVGGameMode::StartDuelPhase(AVGCharacterBase* Challenger, AVGCharacterBase* Target)
{
	if (!Challenger || !Target || !DuelPhaseClass) return;
	
	DuelChallenger = Challenger;
	DuelTarget = Target;
	
	if (DuelPhaseClass)
	{
		// TransitionToPhase를 사용하면 진행하던 미션페이즈가 사라지기 때문에 PushPhase로 시작
		PushPhase(DuelPhaseClass); 
	}
}

void AVGGameMode::CheckAllPlayersReady()
{
	if (bGameHasStarted) return;
	
	if (!GameState || GameState->PlayerArray.Num() == 0) return;
	
	if (GameState->PlayerArray.Num() < MinimumPlayersNeeded) return;
	
	bool bAllReady = true;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(PlayerState);
		if (VGPlayerState && !VGPlayerState->bIsReady)
		{
			bAllReady = false;
			break;
		}
	}
	
	if (bAllReady)
	{
		bGameHasStarted = true;
		UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 전원 레디 완료, 직업 분배"));
		AssignRolesAndStartGame();
	}
}

void AVGGameMode::AssignRolesAndStartGame()
{
	TArray<APlayerState*> Players = GameState->PlayerArray;
	int32 PlayerCount = Players.Num();
	
	TArray<FGameplayTag> RolePool;
	
	RolePool.Add(VigilantRoleTags::Mafia);
	for (int32 i = 1; i < PlayerCount; ++i)
	{
		RolePool.Add(VigilantRoleTags::Citizen);
	}
	
	Algo::RandomShuffle(RolePool);
	
	for (int32 i = 0; i <PlayerCount; ++i)
	{
		AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(Players[i]);
		if (VGPlayerState)
		{
			VGPlayerState->Client_ReceiveRole(RolePool[i]);
			VGPlayerState->AddPlayerTag(RolePool[i]);
		}
	}
	
	if (InitialPhase != nullptr)
	{
		PhaseStack.Last()->ExecutePhaseResult();
	}
}

void AVGGameMode::OnMissionCleared(int32 TimeReducedAmount)
{
	if (PhaseStack.Num() > 0)
	{
		PhaseStack.Last()->OnMissionCleared(TimeReducedAmount);
	}
}

void AVGGameMode::OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim)
{
	if (PhaseStack.Num() > 0)
	{
		PhaseStack.Last()->OnPlayerDeath(Killer,Victim);
	}
}




