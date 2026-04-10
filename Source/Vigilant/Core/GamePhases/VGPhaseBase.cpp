#include "Core/GamePhases/VGPhaseBase.h"
#include "Core/VGGameMode.h"
#include "TimerManager.h"


void UVGPhaseBase::InitializePhase(AVGGameMode* InGameMode)
{
	GameModeRef = InGameMode;
}

void UVGPhaseBase::ProcessVote(AVGPlayerState* Voter, AVGPlayerState* Target)
{
}

void UVGPhaseBase::EnterPhase()
{
	FString PhaseName = GetClass()->GetName();
	UE_LOG(LogTemp, Warning, TEXT("[VGPhaseBase] %s 진입 (EnterPhase)"), *PhaseName);
}

void UVGPhaseBase::ExitPhase()
{
	FString PhaseName = GetClass()->GetName();
	UE_LOG(LogTemp, Warning, TEXT("[VGPhaseBase] %s 종료 (ExitPhase)"), *PhaseName);
	
	if (GameModeRef && PhaseTimerHandle.IsValid())
	{
		GameModeRef->GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	}
}

void UVGPhaseBase::PausePhase()
{
}

void UVGPhaseBase::ResumePhase()
{
}

bool UVGPhaseBase::CheckPhaseEndCondition()
{
	return false;
}

void UVGPhaseBase::ExecutePhaseResult()
{
}

bool UVGPhaseBase::CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target)
{
	return true;
}

bool UVGPhaseBase::CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target)
{
	return true;
}

bool UVGPhaseBase::CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject)
{
	return true;
}

void UVGPhaseBase::OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim)
{
}

void UVGPhaseBase::OnMissionCleared(float TimeReducedAmount)
{
}
