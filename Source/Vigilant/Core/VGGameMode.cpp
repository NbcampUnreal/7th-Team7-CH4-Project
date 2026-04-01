#include "Core/VGGameMode.h"


void AVGGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AVGGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	
}

void AVGGameMode::ChangePhase(TSubclassOf<class UVGPhaseBase> NewPhase)
{
}

void AVGGameMode::AssignRoles()
{
}

void AVGGameMode::CheckWinCondition()
{
}

void AVGGameMode::OnMissionCleared(int32 TimeReducedAmount)
{
}

void AVGGameMode::OnPlayerDeath()
{
}


