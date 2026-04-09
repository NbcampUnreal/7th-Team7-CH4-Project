#include "Core/VGGameMode.h"

#include "EngineUtils.h"
#include "gameframework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Core/GamePhases/VGPhaseBase.h"
#include "Core/VGPlayerState.h"
#include "Core/VGGameState.h"
#include "Algo/RandomShuffle.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"
#include "Interface/VGChatReciveInterface.h"


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
	// 게임 중 유저가 들어왔을 때 예외처리
	if (bGameHasStarted)
	{
		// 최대 인원(6명) 확인
		if (GameState->PlayerArray.Num() <= 6)
		{
			
			NewPlayer->StartSpectatingOnly(); 
			UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 게임 진행 중 난입: 관전자 모드 진입"));
            
			
			return; 
		}
		else
		{
			// 6명을 넘으면 접속안되는 로직 추가 예정
		}
	}
	
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

void AVGGameMode::Logout(AController* Exiting)
{
	AVGPlayerState* ExitingPlayerState = Exiting->GetPlayerState<AVGPlayerState>();
	if (ExitingPlayerState && bGameHasStarted)
	{
		//  나간 사람의 직업 확인
		if (ExitingPlayerState->IsRole(VigilantRoleTags::Mafia))
		{
			UE_LOG(LogTemp, Warning, TEXT("마피아 탈주! 시민 승리."));
			// 게임 종료 로직 추가 예정
		}
		else if (ExitingPlayerState->IsRole(VigilantRoleTags::Citizen))
		{
			UE_LOG(LogTemp, Warning, TEXT("시민 %s 탈주!"), *ExitingPlayerState->VGPlayerName);
			// 전체 공지해야함
		}
	}

	// 레디 중인 사람이 나갔을 때를 위한 체크 
	if (!bGameHasStarted)
	{
		// 다시 전인원 레디 여부 판정
		CheckAllPlayersReady();
	}
	
	Super::Logout(Exiting);
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
			FGameplayTag AssignedRole = RolePool[i];
			VGPlayerState->SecretRoleTag = AssignedRole;
			// 직업 부여받은 유저에게만 자기 직업 알려줌
			VGPlayerState->Client_ReceiveRole(AssignedRole);
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

void AVGGameMode::ProcessChatMessage(const FString& SenderName, const FString& Message)
{
	// 월드에있는 플레이어 전부에게 뿌리기
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (IVGChatReciveInterface* ChatReceiveInterface = Cast<IVGChatReciveInterface>(PlayerController))
		{
			//TODO: 가져온 이름과 메세지를 합쳐서 하나의 멧쎄지로만들기
			FString NickNameMessage = SenderName + TEXT(" : ") + Message; // 임시
			
			//이 함수 구현은 플레이어 컨트롤러에 있어요~
			ChatReceiveInterface->ReceiveChatMessage(NickNameMessage);
		}
	}
}

void AVGGameMode::OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim)
{
	if (PhaseStack.Num() > 0)
	{
		PhaseStack.Last()->OnPlayerDeath(Killer,Victim);
	}
}




