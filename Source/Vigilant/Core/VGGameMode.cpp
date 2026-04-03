#include "Core/VGGameMode.h"
#include "gameframework/PlayerController.h"
#include "Core/GamePhases/VGPhaseBase.h"


void AVGGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 게임 시작 페이즈 세팅"));
	
	if (InitialPhase != nullptr)
	{
		PushPhase(InitialPhase);
	}
}

void AVGGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	
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
		PhaseStack.Last()->ResumePhase();
	}
}



void AVGGameMode::AssignRoles()
{
}

void AVGGameMode::CheckWinCondition()
{
}

void AVGGameMode::StartDuelPhase(AVGCharacterBase* Challenger, AVGCharacterBase* Target)
{
	if (!Challenger || !Target || !DuelPhaseClass) return;
	
	DuelChallenger = Challenger;
	DuelTarget = Target;
	
	PushPhase(DuelPhaseClass);
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




