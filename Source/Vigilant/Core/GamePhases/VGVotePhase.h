#pragma once

#include "CoreMinimal.h"
#include "VGPhaseBase.h"
#include "VGVotePhase.generated.h"

class AVGPlayerState;

UCLASS(Blueprintable)
class VIGILANT_API UVGVotePhase : public UVGPhaseBase
{
	GENERATED_BODY()
	
public:
	
	// 투표 결과에 따른 보스 스탯 변동량
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Vigilant|Vote")
	float BossStatChangeAmount = 0.2f;
	
	virtual void EnterPhase() override;
	virtual void ExitPhase() override;
	virtual void ExecutePhaseResult() override;
	
	virtual bool CanPlayerInteract(AVGCharacterBase* Player, AActor* InteractableObject) override;
	virtual bool CanPlayerAttack(AVGCharacterBase* Attacker, AVGCharacterBase* Target) override;
	virtual bool CanPlayerTakeDamage(AActor* DamageCauser, AVGCharacterBase* Target) override;
	
	void OnVoteTimeUp();
	
protected:
	// 투표 내역 저장소 (Key: 투표한 사람, Value: 지목당한 사람)
	UPROPERTY(Transient)
	TMap<AVGPlayerState*, AVGPlayerState*> PlayerVotes;
	
public:
	// 클라이언트가 서버로 투표 보낼 때 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Vote")
	virtual void ProcessVote(AVGPlayerState* Voter, AVGPlayerState* VotedTarget) override;

	// 시간이 다 되었을 때 표를 집계하고 결과를 계산하는 함수
	void CalculateVoteResult();
};
