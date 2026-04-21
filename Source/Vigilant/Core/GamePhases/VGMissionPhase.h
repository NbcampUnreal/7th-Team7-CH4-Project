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
	virtual bool CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target);
	virtual bool CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target);
	virtual void OnMissionCleared(float TimeReducedAmount) override;
	
	void OnMissionTimeUp();
	
protected:
	// 막고라 페이즈 넘어갔을 때 페이즈 시간 재계산용
	float PauseBeginServerTime = 0.0f;
	
	// 막고라 페이즈에서 -1 로 초기화되기전 원래 미션 페이즈 시간 저장용
	float SavedPhaseStartTime = 0.0f;
	float SavedPhaseEndTime = 0.0f;
};
