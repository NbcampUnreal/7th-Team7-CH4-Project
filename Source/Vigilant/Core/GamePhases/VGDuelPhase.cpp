#include "VGDuelPhase.h"
#include "Core/VGGameMode.h"
#include "Character/VGCharacterBase.h"
#include "Core/VGPlayerState.h"
#include "Common/VGGameplayTags.h"
#include "Kismet/GameplayStatics.h"

void UVGDuelPhase::EnterPhase()
{
	Super::EnterPhase();
	
	if (!GameModeRef) return;
	
	AVGCharacterBase* Player1 = GameModeRef->GetDuelChallenger();
	AVGCharacterBase* Player2 = GameModeRef->GetDuelTarget();
	
	if (Player1 && Player2)
	{
		// 나중에 로직 추가 예정
	}
}

void UVGDuelPhase::ExitPhase()
{
	
	if (GameModeRef)
	{
		GameModeRef->ClearDuelParticipants();
		
	}
	
	Super::ExitPhase();
}

void UVGDuelPhase::ExecutePhaseResult()
{
	if (GameModeRef)
	{
		GameModeRef->TransitionToPhase(nullptr);
	}
}

bool UVGDuelPhase::CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject)
{
	if (!InteractableObject)
	{
		return false;
	}
	if (InteractableObject->IsA(AVGCharacterBase::StaticClass()))
	{
		return false;
	}
	
	return true;
}

void UVGDuelPhase::OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim)
{
	if (!GameModeRef) return;
	
	if (Victim == GameModeRef->GetDuelChallenger() || Victim == GameModeRef->GetDuelTarget())
	{
		ExecutePhaseResult();
	}
}
