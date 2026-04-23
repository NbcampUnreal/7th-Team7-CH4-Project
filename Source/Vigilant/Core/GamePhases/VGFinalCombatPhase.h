#pragma once

#include "CoreMinimal.h"
#include "VGPhaseBase.h"
#include "VGFinalCombatPhase.generated.h"

class AVGCharacterBase;

UCLASS(Blueprintable)
class VIGILANT_API UVGFinalCombatPhase : public UVGPhaseBase
{
	GENERATED_BODY()
public:
	UVGFinalCombatPhase();
	
	virtual void EnterPhase() override;
	virtual void ExitPhase() override;
	
	virtual void ExecutePhaseResult() override;
	
	virtual bool CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject) override;
	virtual bool CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target) override;
	virtual bool CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target) override;
	
	virtual void OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim) override;
	
	// 블루프린트에서 할당할 보스 캐릭터 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Classes")
	TSubclassOf<AVGCharacterBase> BossCharacterClass;
};
