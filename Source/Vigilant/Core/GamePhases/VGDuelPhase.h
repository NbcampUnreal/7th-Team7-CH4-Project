#pragma once

#include "CoreMinimal.h"
#include "VGPhaseBase.h"
#include "VGDuelPhase.generated.h"

class AVGCharacterBase;

UCLASS(Blueprintable)
class VIGILANT_API UVGDuelPhase : public UVGPhaseBase
{
	GENERATED_BODY()
	
public:
	
	virtual void EnterPhase() override;
	virtual void ExitPhase() override;
	virtual void ExecutePhaseResult() override;
	
	virtual bool CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject) override;
	virtual bool CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target) override;
	
	virtual void OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim) override;
	
};
