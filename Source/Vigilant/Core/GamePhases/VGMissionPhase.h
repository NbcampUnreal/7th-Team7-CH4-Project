#pragma once

#include "CoreMinimal.h"
#include "Core/GamePhases/VGPhaseBase.h"
#include "VGMissionPhase.generated.h"


UCLASS(Blueprintable)
class VIGILANT_API UVGMissionPhase : public UVGPhaseBase
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Vigilant|Phase")
	TSubclassOf<class UVGPhaseBase> NextPhaseClass;
	
	virtual void EnterPhase() override;
	virtual void ExitPhase() override;
	
	virtual void ExecutePhaseResult() override;
	
	virtual void PausePhase() override;
	virtual void ResumePhase() override;
	
	virtual void OnMissionCleared(int32 TimeReducedAmount) override;
	
	void OnMissionTimeUp();
};
