#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "VGGameMode.generated.h"

class AVGCharacterBase;
class UVGPhaseBase;

UCLASS()
class VIGILANT_API AVGGameMode : public AGameMode
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
	
	// 현제 게임의 페이즈 객체
	UPROPERTY(Transient)
	TArray<UVGPhaseBase*> PhaseStack;
	
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Phase")
	TSubclassOf<class UVGPhaseBase> InitialPhase;
	
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Phase")
	TSubclassOf<class UVGPhaseBase> DuelPhaseClass;
	
public:
	
	UPROPERTY(Transient)
	AVGCharacterBase* DuelChallenger;
	
	UPROPERTY(Transient)
	AVGCharacterBase* DuelTarget;
	
	virtual void PostLogin(APlayerController *NewPlayer) override;
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void PushPhase(TSubclassOf<class UVGPhaseBase> NewPhase);
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void PopPhase();
	
	void CheckWinCondition();
	
	void StartDuelPhase(AVGCharacterBase* Challenger, AVGCharacterBase* Target);
	
	void CheckAllPlayersReady();
	
	void AssignRolesAndStartGame();
	
	// 이벤트 중개소
	void OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim);
	void OnMissionCleared(int32 TimeReducedAmount);
	
	//채팅 관련 함수 -입력메시지를 뿌려주는 역할 -김형백
	void ProcessChatMessage(const FString& SenderName, const FString& Message);
};
