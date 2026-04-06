#pragma once

#include "CoreMinimal.h"
#include "VGPhaseBase.h"
#include "VGVotePhase.generated.h"



UCLASS(Blueprintable)
class VIGILANT_API UVGVotePhase : public UVGPhaseBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Vigilant|Phase")
	TSubclassOf<class UVGPhaseBase> NextPhaseClass;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Vigilant|Phase")
	float PhaseTime = 20.0f;
	
	virtual void EnterPhase() override;
	virtual void ExitPhase() override;
	virtual void ExecutePhaseResult() override;
	
	virtual bool CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target) override;
	virtual bool CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target) override;
	
	void OnVoteTimeUp();
};
