#pragma once

#include "CoreMinimal.h"
#include "Combat/VGAttackExecution.h"
#include "VGMeleeExecution.generated.h"

class UNiagaraComponent;

UCLASS(NotBlueprintable)
class VIGILANT_API UVGMeleeExecution : public UVGAttackExecution
{
	GENERATED_BODY()

public:
	virtual void StartAttack() override;
	virtual void TickAttack() override;
	virtual void StopAttack() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float AttackRadius = 20.0f;
	
private:
	// Hit Detection State
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> HitActorsThisSwing;
	
	UPROPERTY(Transient)
	TMap<FName, FVector> PreviousSocketLocations;
	
	// VFX
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> SpawnedTrail;
};
