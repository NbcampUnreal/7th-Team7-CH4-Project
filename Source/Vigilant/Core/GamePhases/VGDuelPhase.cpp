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
	
	AVGCharacterBase* Player1 = GameModeRef->DuelChallenger;
	AVGCharacterBase* Player2 = GameModeRef->DuelTarget;
	
	if (Player1 && Player2)
	{
		// 나중에 로직 추가 예정
	}
}

void UVGDuelPhase::ExitPhase()
{
	
	if (GameModeRef)
	{
		GameModeRef->DuelChallenger = nullptr;
		GameModeRef->DuelTarget = nullptr;
	}
	
	Super::ExitPhase();
}

void UVGDuelPhase::ExecutePhaseResult()
{
	if (GameModeRef)
	{
		GameModeRef->PopPhase();
	}
}

bool UVGDuelPhase::CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject)
{
	// [Fix] null 체크 추가 + 불필요한 이중 괄호 제거
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
	
	if (Victim == GameModeRef->DuelChallenger || Victim == GameModeRef->DuelTarget)
	{
		ExecutePhaseResult();
	}
}
