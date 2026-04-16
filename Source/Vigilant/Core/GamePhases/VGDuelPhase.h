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
	
};
