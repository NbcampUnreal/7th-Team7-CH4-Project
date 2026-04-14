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
		if (VGPlayerState->EntryIndex == 0)
		{
			ConnectedPlayerCount++;
			VGPlayerState->EntryIndex = ConnectedPlayerCount;
			UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 플레이어 번호 발급 완료: %d번"), VGPlayerState->EntryIndex);
		}
		// 내 번호표에 맞는 PlayerStart를 찾아 엔진에게 넘겨줌
		if (APlayerStart** FoundStart = CachedJailSpawns.Find(VGPlayerState->EntryIndex))
		{
			return *FoundStart;
		}
	}

	// 만약 감옥을 못 찾거나 치명적인 에러 시 원래 엔진 로직 값
	return Super::ChoosePlayerStart_Implementation(Player);
}

FString AVGGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal)
{
	FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	
	if (!ErrorMessage.IsEmpty())
	{
		return ErrorMessage;
	}

	// 2. Options 문자열에서 "Name" 키값을 파싱
	FString ParsedName = UGameplayStatics::ParseOption(Options, TEXT("VGName"));
    
	if (!ParsedName.IsEmpty())
	{
		// 3. PlayerController를 통해 PlayerState를 가져와 이름 세팅
		if (AVGPlayerState* VGPlayerState = NewPlayerController->GetPlayerState<AVGPlayerState>())
		{
			VGPlayerState->VGPlayerName = ParsedName;
			UE_LOG(LogTemp, Log, TEXT("[VGGameMode] InitNewPlayer - 접속자 이름 설정: %s"), *ParsedName);
		}
	}

	return ErrorMessage;
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
	
	UE_LOG(LogTemp, Log, TEXT("[VGGameMode] 플레이어 접속 완료. 배정된 번호: %d"), NewPlayer->GetPlayerState<AVGPlayerState>()->EntryIndex);
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
	// 마피아 수가 시민 수보다 같거나 많아지면 마피아 승리
	else if (AliveMafia > AliveCitizen)
	{
		UE_LOG(LogTemp, Warning, TEXT("마피아 측 승리! 시민들이 제압되었습니다."));
		WinnerTeam = VigilantRoleTags::Mafia;
		bGameOver = true;
	}
	
	if (bGameOver)
	{

		// 아마 여기서 승리 팀 정보 저장해서 UI에서 쓸 듯함

		// 다음 페이즈로 보냄
		if (PhaseStack.Num() > 0)
		{
			PhaseStack.Last()->ExecutePhaseResult();
		}
	}
}

void AVGGameMode::ResetGameStatus()
{
	// 게임모드 관리 변수 초기화
	bGameHasStarted = false;
	ClearDuelParticipants();

	// 전광판 글로벌 수치 초기화
	if (AVGGameState* VGGameState = GetGameState<AVGGameState>())
	{
		VGGameState->BossNerfRate = 1.0f; // 보스 스탯 복구
		// 후에 승리팀 저장되어있으면 그것도 초기화
	}

	// 전체 플레이어 개별 초기화 순회
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(PlayerState);
		if (!VGPlayerState) continue;

		// PlayerState 데이터 리셋
		VGPlayerState->bIsReady = false; 
		VGPlayerState->SecretRoleTag = FGameplayTag::EmptyTag; // 직업 압수
		VGPlayerState->PlayerStatusTags.Reset(); // Dead, Duel 등의 상태 태그 컨테이너를 아예 깨끗하게 비움

		// Character 스탯 복구 및 감옥 텔레포트
		if (AVGCharacterBase* VGCharacter = Cast<AVGCharacterBase>(VGPlayerState->GetPawn()))
		{
			// 후에 캐릭터 파일 확인해서 캐릭터 스탯 및 죽음 원복

			// 자기 번호표에 맞는 감옥으로 이송
			/*
			int32 SpawnIndex = VGPlayerState->EntryIndex - 1;
			if (JailSpawnPoints.IsValidIndex(SpawnIndex) && JailSpawnPoints[SpawnIndex])
			{
				Character->SetActorLocationAndRotation(
					JailSpawnPoints[SpawnIndex]->GetActorLocation(),
					JailSpawnPoints[SpawnIndex]->GetActorRotation()
				);
			}
			*/
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

void AVGGameMode::OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim)
{
	if (PhaseStack.Num() > 0)
	{
		PhaseStack.Last()->OnPlayerDeath(Killer,Victim);
	}
}




