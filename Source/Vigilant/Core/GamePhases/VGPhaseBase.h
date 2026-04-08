#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/VGGameState.h"
#include "VGPhaseBase.generated.h"

class AVGGameMode;
class AVGCharacterBase;

UCLASS(Blueprintable,Abstract)
class VIGILANT_API UVGPhaseBase : public UObject
{
	GENERATED_BODY()
	
protected:
	// 총괄 게임모드
	UPROPERTY(Transient)
	AVGGameMode* GameModeRef;
	
	//테스트용 타이머
	FTimerHandle PhaseTimerHandle;
	
public:
	// 현재 페이즈 표시용
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Vigilant|Phase")
	FGameplayTag PhaseTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Vigilant|Phase")
	float PhaseDuration = 60.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Vigilant|Phase")
	bool bHasTimeLimit = true;
	
	virtual void  InitializePhase(AVGGameMode* InGameMode);
	
	// 페이즈 생명주기 관련 함수
	virtual void EnterPhase();
	virtual void ExitPhase();
	
	// 페이즈 중 다른 페이즈 난입할 때 호출 (막고라용)
	virtual void PausePhase();
	virtual void ResumePhase();
	
	// 페이즈 종료 및 결과 관련 함수
	virtual bool CheckPhaseEndCondition();
	virtual void ExecutePhaseResult();
	
	// 게임 규칙 관련 함수
	virtual bool CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target);
	virtual bool CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target);
	virtual bool CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject);
	
	// 이벤트 수신용
	virtual void OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim);
	virtual void OnMissionCleared(int32 TimeReducedAmount);
	
};
