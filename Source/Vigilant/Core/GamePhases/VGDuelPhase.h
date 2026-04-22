#pragma once

#include "CoreMinimal.h"
#include "VGPhaseBase.h"
#include "VGDuelPhase.generated.h"

class AVGCharacterBase;

UCLASS(Blueprintable)
class VIGILANT_API UVGDuelPhase : public UVGPhaseBase
{
	GENERATED_BODY()
	
public:
	
	virtual void EnterPhase() override;
	virtual void ExitPhase() override;
	virtual void ExecutePhaseResult() override;
	
	virtual bool CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject) override;
	virtual bool CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target);
	virtual bool CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target) override;
	
	virtual void OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim) override;
	
protected:	
	// 플레이어 복귀 위치 저장용 맵
	UPROPERTY(Transient)
	TMap<class AVGPlayerState*, FTransform> OriginalTransforms;
    
	// 아레나 스폰 포인트들을 임시 저장할 배열
	UPROPERTY(Transient)
	TArray<AActor*> ParticipantStarts;
	UPROPERTY(Transient)
	TArray<AActor*> SpectatorStarts;
	
	bool bIsDuelFinished = false;

	// 각 참가자의 체력 변화를 감지할 델리게이트 함수
	UFUNCTION()
	void OnChallengerHPChanged(float NewHP, float MaxHP);
	UFUNCTION()
	void OnTargetHPChanged(float NewHP, float MaxHP);

	// 20% 미만이 되었을 때 호출할 공통 패배 처리 함수
	void HandleDuelDefeat(AVGCharacterBase* DefeatedPlayer);
};
