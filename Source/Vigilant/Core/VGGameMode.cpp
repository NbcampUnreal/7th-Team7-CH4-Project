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
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Mission/VGMissionSubsystem.h"


AVGGameMode::AVGGameMode()
{
	bUseSeamlessTravel = true;
}

void AVGGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 게임 시작 페이즈 세팅"));
	
	if (UWorld* World = GetWorld())
	{
		if (UVGMissionSubsystem* VGMissionSubsystem = World->GetSubsystem<UVGMissionSubsystem>())
		{
			VGMissionSubsystem->OnMissionClearTimeReward.AddDynamic(this, &AVGGameMode::HandleMissionClear);
		}
	}
	
	if (InitialPhase)
	{
		PushPhase(InitialPhase); 
	}
	
}

void AVGGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	// 게임이 이미 시작되었을 때(후에 관전자 시점으로 수정할 수도 있음)
	if (bGameHasStarted)
	{
		ErrorMessage = TEXT("Game is already in progress.");
		return;
	}

	// 서버에 이미 6명이 있을 때
	if (ConnectedPlayerCount >= 6)
	{
		ErrorMessage = TEXT("Server is full (Max 6 Players).");
		return;
	}
}

void AVGGameMode::ClearDuelParticipants()
{
	DuelChallenger = nullptr;
	DuelTarget = nullptr;
}

AActor* AVGGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// 서버 열리고 누군가 처음 스폰을 시도할 때 캐시 맵 세팅
	if (CachedJailSpawns.IsEmpty())
	{
		TArray<AActor*> FoundStarts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundStarts);
		
		UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 월드에서 찾은 PlayerStart 개수: %d개"), FoundStarts.Num());
		
		for (AActor* StartActor : FoundStarts)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(StartActor))
			{
				FString TagString = PlayerStart->PlayerStartTag.ToString();
				if (TagString.StartsWith(TEXT("Jail")))
				{
					FString NumberString = TagString.Replace(TEXT("Jail"), TEXT(""));
					if (NumberString.IsNumeric())
					{
						int32 SpawnIndex = FCString::Atoi(*NumberString);
						CachedJailSpawns.Add(SpawnIndex, PlayerStart);
					}
				}
			}
		}
	}
	
	// 현재 스폰을 기다리는 플레이어의 번호표 확인
	if (AVGPlayerState* VGPlayerState = Player->GetPlayerState<AVGPlayerState>())
	{
		int32 MyIndex = (VGPlayerState->EntryIndex == 0) ? AssignPlayerSlot(VGPlayerState) : VGPlayerState->EntryIndex;
		UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 스폰 위치 탐색 중 - 번호: %d"), MyIndex);
        
		if (APlayerStart** FoundStart = CachedJailSpawns.Find(MyIndex))
		{
			return *FoundStart;
		}
	}

	// 만약 감옥을 못 찾거나 치명적인 에러 시 원래 엔진 로직 값
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AVGGameMode::OnMissionTimeUpdated()
{
	if (AVGGameState* VGGameState = GetGameState<AVGGameState>())
	{
		float RemainingTime = VGGameState->GetRemainingPhaseTime();
		//구현되면 주석빼기
		float ElapsedTime = VGGameState->GetElapsedTime();
		OnMissionTimeRemainingChanged.Broadcast(ElapsedTime, RemainingTime);
	}
	
}

FString AVGGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options,
                                   const FString& Portal)
{
	FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	
	if (!ErrorMessage.IsEmpty())
	{
		return ErrorMessage;
	}
	
	if (AVGPlayerState* VGPlayerState = NewPlayerController->GetPlayerState<AVGPlayerState>())
	{
		// 이미 부여받았어야하지만 안전을 위해 또 호출
		AssignPlayerSlot(VGPlayerState);
		
		FString PlayerName = UGameplayStatics::ParseOption(Options, TEXT("VGName"));
		if (PlayerName.IsEmpty())
		{
			PlayerName = TEXT("Unknown_Player");
		}
		
		VGPlayerState->SetVGPlayerName(PlayerName.IsEmpty() ? TEXT("Unknown_Player") : PlayerName);
		
		UE_LOG(LogTemp, Log, TEXT("[VGGameMode] InitNewPlayer - 접속 완료: %s (슬롯: %d)"), 
			*PlayerName, VGPlayerState->EntryIndex);
	}
    
	return ErrorMessage;
}

void AVGGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	ConnectedPlayerCount++;
	UE_LOG(LogTemp, Log, TEXT("[VGGameMode] 플레이어 접속 완료. 배정된 번호: %d"), NewPlayer->GetPlayerState<AVGPlayerState>()->EntryIndex);
}

void AVGGameMode::Logout(AController* Exiting)
{
	AVGPlayerState* ExitingPlayerState = Exiting->GetPlayerState<AVGPlayerState>();
	if (ExitingPlayerState && bGameHasStarted)
	{
		// 해당되는 슬롯 비우기
		int32 ReleasedIndex = ExitingPlayerState->EntryIndex - 1;
		if (ReleasedIndex >= 0 && ReleasedIndex < 6)
		{
			bSlotOccupied[ReleasedIndex] = false;
		}
		
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
	
	ConnectedPlayerCount = FMath::Max(0, ConnectedPlayerCount - 1);

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
			VGGameState->SetCurrentPhaseTag(CreatedPhase->PhaseTag); 
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
			VGGameState->SetCurrentPhaseTag(ResumedPhase->PhaseTag);
		}
	}
}



void AVGGameMode::CheckWinCondition()
{
	if (!bGameHasStarted) return;
	
	int32 AliveCitizen = 0;
	int32 AliveMafia = 0;
	
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(PlayerState);
		if (VGPlayerState)
		{
			if (VGPlayerState->HasPlayerTag(VigilantRoleTags::Dead)) continue;
			
			if (VGPlayerState->IsRole(VigilantRoleTags::Mafia))
			{
				AliveMafia++;
			}
			else if (VGPlayerState->IsRole(VigilantRoleTags::Citizen))
			{
				AliveCitizen++;
			}
		}
	}
	
	bool bGameOver = false;
	FGameplayTag WinnerTeam;
	
	// 마피아 죽으면 시민 승리
	if (AliveMafia == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("시민 측 승리! 모든 마피아가 소탕되었습니다."));
		WinnerTeam = VigilantRoleTags::Citizen;
		bGameOver = true;
	}
	// 마피아 수가 시민 수보다 많아지면 마피아 승리
	else if (AliveMafia > AliveCitizen)
	{
		UE_LOG(LogTemp, Warning, TEXT("마피아 측 승리! 시민들이 제압되었습니다."));
		WinnerTeam = VigilantRoleTags::Mafia;
		bGameOver = true;
	}
	
	if (bGameOver)
	{
		if (PhaseStack.Num() > 0)
		{
			PhaseStack.Last()->ExecutePhaseResult();
		}
	}
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

void AVGGameMode::NotifyPhaseCompleted(class UVGPhaseBase* CompletedPhase)
{
	if (CompletedPhase->PhaseTag.MatchesTag(VigilantPhaseTags::PhaseCombat)) 
	{
		HandleMatchFinished();
		return;
	}
	
	if (CompletedPhase->NextPhaseClass)
	{
		TransitionToPhase(CompletedPhase->NextPhaseClass);
	}
	else
	{
		TransitionToPhase(nullptr);
	}
}

bool AVGGameMode::CanPlayerAttack_Implementation(class AVGCharacterBase* Attacker)
{
	if (PhaseStack.Num() > 0)
	{
		return PhaseStack.Last()->CanPlayerAttack(Attacker, nullptr);
	}
	return true;
}

bool AVGGameMode::CanPlayerInteract_Implementation(class AVGCharacterBase* Interactor, AActor* Target)
{
	if (PhaseStack.Num() > 0 && PhaseStack.Last())
	{
		return PhaseStack.Last()->CanPlayerInteract(Interactor, Target);
	}
	return false;
}

bool AVGGameMode::CanPlayerTakeDamage_Implementation(AActor* DamageCauser, class AVGCharacterBase* Target)
{
	if (PhaseStack.Num() > 0 && PhaseStack.Last())
	{
		return PhaseStack.Last()->CanPlayerTakeDamage(DamageCauser, Target);
	}
	return false;
}


void AVGGameMode::HandleMatchFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 모든 페이즈 종료. 서버 트래블 초기화 시작"));
	GetWorld()->ServerTravel(TEXT("/Game/Vigilant/Levels/MainLevel_WP?listen"));
}

int32 AVGGameMode::AssignPlayerSlot(class AVGPlayerState* VGPlayerState)
{
	if (!VGPlayerState || VGPlayerState->EntryIndex > 0) return VGPlayerState ? VGPlayerState->EntryIndex : -1;

	// 0 ~ 5 슬롯 중 비어있는 곳 찾기
	for (int32 i = 0; i < 6; i++)
	{
		if (!bSlotOccupied[i])
		{
			bSlotOccupied[i] = true;
			// 1 ~ 6 까지 숫자를 넣어야하므로 i + 1 값 대입
			VGPlayerState->EntryIndex = i + 1; 
			UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 실시간 번호 부여 완료: %d번"), VGPlayerState->EntryIndex);
			return VGPlayerState->EntryIndex;
		}
	}
	return -1;
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

void AVGGameMode::NotifyPlayerDeath_Implementation(class AVGCharacterBase* Killer, class AVGCharacterBase* Victim)
{
	if (PhaseStack.Num() > 0 && PhaseStack.Last())
	{
		PhaseStack.Last()->OnPlayerDeath(Killer, Victim);
	}
}

void AVGGameMode::SubmitVote(AVGPlayerState* Voter, int32 TargetIndex)
{
	if (PhaseStack.Num() > 0)
	{
		AVGPlayerState* TargetPlayer = nullptr;

		// EntryIndex로 대상 색출
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(PlayerState);
			if (VGPlayerState && VGPlayerState->EntryIndex == TargetIndex)
			{
				TargetPlayer = VGPlayerState;
				break;
			}
		}

		//찾았다면 페이즈에게 투표 처리
		if (TargetPlayer)
		{
			PhaseStack.Last()->ProcessVote(Voter, TargetPlayer);
		}
		else
		{
			// 만약 그 사이에 대상자가 탈주해서 못 찾았을 경우의 안전장치
			UE_LOG(LogTemp, Error, TEXT("[VGGameMode] 투표 대상인 %d번 유저를 찾을 수 없습니다."), TargetIndex);
		}
	}
}

void AVGGameMode::HandleMissionClear(float ReduceTime)
{
	if (PhaseStack.Num() > 0)
	{
		if (PhaseStack.Last()->PhaseTag == VigilantPhaseTags::PhaseMission)
		{
			PhaseStack.Last()->OnMissionCleared(ReduceTime);
			
			//김형백- 남은시간, 경과시간 전달 함수
			OnMissionTimeUpdated();
		}
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
			// 보낸 사람 이름 + 보낸 메시지 형식으로 묶음
			FString FormattedMessage = FString::Printf(TEXT("[%s] : %s"), *SenderName, *Message);
			
			//이 함수 구현은 플레이어 컨트롤러에 있어요~
			ChatReceiveInterface->ReceiveChatMessage(FormattedMessage);
		}
	}
}

void AVGGameMode::RequestDuelPhase_Implementation(AVGCharacterBase* Challenger, AVGCharacterBase* Target)
{
	// 해당되는 사람이 나갔거나 사라졌으면 요청 안받음
	if (!Challenger || !Target) 
	{
		return;
	}
	
	if (PhaseStack.Num() > 0)
	{
		if(PhaseStack.Last()->PhaseTag == VigilantPhaseTags::PhaseMission)
		{
			StartDuelPhase(Challenger, Target);
			UE_LOG(LogTemp,Warning,TEXT("StartDuelPhase 함수 실행") );
		}
		// 디버그용
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[VGGameMode] 현재 미션 페이즈가 아니므로 막고라 요청이 무시되었습니다."));
		}
	}
}





