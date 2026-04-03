#include "VGFinalCombatPhase.h"
#include "Core/VGGameMode.h"
#include "Core/VGPlayerState.h"
#include "Character/VGCharacterBase.h"
#include "Common/VGGameplayTags.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"

void UVGFinalCombatPhase::EnterPhase()
{
	Super::EnterPhase();
	UE_LOG(LogTemp, Warning, TEXT("[VGFinalCombatPhase] 최후의 전투 시작"));
}

void UVGFinalCombatPhase::ExitPhase()
{
	Super::ExitPhase();
}

void UVGFinalCombatPhase::ExecutePhaseResult()
{
	if (GameModeRef)
	{
		GameModeRef->CheckWinCondition();
	}
}

bool UVGFinalCombatPhase::CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target)
{
	AVGCharacterBase* Attacker = Cast<AVGCharacterBase>(DamageCauser);
	
	if (Attacker && Target)
	{
		AVGPlayerState* AttackerPlayerState = Cast<AVGPlayerState>(Attacker);
		AVGPlayerState* TargetPlayerState = Cast<AVGPlayerState>(Target);
		
		if (AttackerPlayerState && TargetPlayerState)
		{
			bool bAttackerIsCitizen =  AttackerPlayerState->PlayerTags.HasTag(VigilantRoleTags::Citizen);
			bool bTargetIsCitizen = TargetPlayerState->PlayerTags.HasTag(VigilantRoleTags::Citizen);
			
			if (bAttackerIsCitizen && bTargetIsCitizen)
			{
				return false;
			}
		}
	}
	
	return true;
}

void UVGFinalCombatPhase::OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim)
{
	ExecutePhaseResult();
}
