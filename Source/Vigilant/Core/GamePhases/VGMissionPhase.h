#pragma once

#include "CoreMinimal.h"
#include "Core/GamePhases/VGPhaseBase.h"
#include "VGMissionPhase.generated.h"


UCLASS(Blueprintable)
class VIGILANT_API UVGMissionPhase : public UVGPhaseBase
{
	GENERATED_BODY()
	
public:
	
	virtual void EnterPhase() override;
	virtual void ExitPhase() override;
	
	virtual void ExecutePhaseResult() override;
	
	virtual void PausePhase() override;
	virtual void ResumePhase() override;
	
	virtual bool CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject) override;
	virtual void OnMissionCleared(float TimeReducedAmount) override;
	
	void OnMissionTimeUp();
};
