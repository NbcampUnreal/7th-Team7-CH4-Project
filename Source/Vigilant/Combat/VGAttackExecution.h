#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VGAttackExecution.generated.h"

class UVGCombatComponent;

UCLASS(Abstract, Blueprintable, EditInlineNew)
class VIGILANT_API UVGAttackExecution : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(UVGCombatComponent* InCombatComponent);
	
	virtual void StartAttack() {}
	virtual void TickAttack() {}
	virtual void StopAttack() {}
	
	virtual UWorld* GetWorld() const override;
	
protected:
	TWeakObjectPtr<UVGCombatComponent> CombatComponent;
	
};
