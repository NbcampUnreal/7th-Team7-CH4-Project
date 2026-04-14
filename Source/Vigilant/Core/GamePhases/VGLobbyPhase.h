#pragma once

#include "CoreMinimal.h"
#include "VGPhaseBase.h"
#include "VGLobbyPhase.generated.h"

UCLASS()
class VIGILANT_API UVGLobbyPhase : public UVGPhaseBase
{
	GENERATED_BODY()
	
public:

	// 페이즈 생명주기
	virtual void EnterPhase() override;
	virtual void ExitPhase() override;
	virtual void ExecutePhaseResult() override;

	// 로비에서 플레이어 움직임 차단
	virtual bool CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target) override;
	virtual bool CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target) override;
	virtual bool CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject) override;
};
