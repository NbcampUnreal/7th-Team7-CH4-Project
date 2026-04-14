#include "Combat/VGAttackExecution.h"
#include "Character/Component/VGCombatComponent.h"

void UVGAttackExecution::Initialize(UVGCombatComponent* InCombatComponent)
{
	CombatComponent = InCombatComponent;
}

UWorld* UVGAttackExecution::GetWorld() const
{
	if (CombatComponent.IsValid())
	{
		return CombatComponent->GetWorld();
	}
	
	return nullptr;
}


