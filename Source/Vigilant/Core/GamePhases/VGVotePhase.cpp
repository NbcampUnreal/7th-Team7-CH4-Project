#include "VGVotePhase.h"
#include "Core/VGGameMode.h"
#include "TimerManager.h"

void UVGVotePhase::EnterPhase()
{
	Super::EnterPhase();
	
	if (GameModeRef)
	{
		GameModeRef->GetWorldTimerManager().SetTimer(
			PhaseTimerHandle, 
			this, 
			&UVGVotePhase::OnVoteTimeUp, 
			PhaseTime, 
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
	if (GameModeRef && NextPhaseClass)
	{
		GameModeRef->TransitionToPhase(NextPhaseClass);
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
	ExecutePhaseResult();
}
