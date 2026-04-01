#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VGCombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VIGILANT_API UVGCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVGCombatComponent();
	
	// TODO: 캐릭터에서 Enhanced Input에 바인딩
	void TryLightAttack();

protected:
	// TODO: 실제 공격 로직 (콤보 확장)
	void PerformAttack(bool bIsHeavy);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryAttack(bool bIsHeavy);
};
