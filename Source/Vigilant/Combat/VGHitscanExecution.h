#pragma once

#include "CoreMinimal.h"
#include "Combat/VGAttackExecution.h"
#include "VGHitscanExecution.generated.h"

UCLASS(NotBlueprintable)
class VIGILANT_API UVGHitscanExecution : public UVGAttackExecution
{
	GENERATED_BODY()
	
public:
	virtual void StartAttack() override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Hitscan")
	float TraceDistance = 5000.0f;
};
