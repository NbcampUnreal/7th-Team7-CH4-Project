#pragma once

#include "CoreMinimal.h"
#include "VGPlayerState.h"
#include "GameFramework/GameMode.h"
#include "Core/Interface/VGGameModeInterface.h"
#include "VGGameMode.generated.h"

class AVGCharacterBase;
class UVGPhaseBase;

UCLASS()
class VIGILANT_API AVGGameMode : public AGameMode, public IVGGameModeInterface
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
	
	// 현제 게임의 페이즈 객체
	UPROPERTY(Transient)
	TArray<UVGPhaseBase*> PhaseStack;
	// 감옥 스폰포인트 저장
	UPROPERTY(Transient)
	TMap<int32, class APlayerStart*> CachedJailSpawns;
	
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Phase")
	TSubclassOf<class UVGPhaseBase> InitialPhase;
	
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Phase")
	TSubclassOf<class UVGPhaseBase> DuelPhaseClass;
	
	UPROPERTY(Transient)
	AVGCharacterBase* DuelChallenger;
	
	UPROPERTY(Transient)
	AVGCharacterBase* DuelTarget;
	
	int32 ConnectedPlayerCount = 0;
	
	// 이미 게임이 시작되었는지 체크용
	bool bGameHasStarted = false;
	
public:
	
	void ClearDuelParticipants();
	
	AVGCharacterBase* GetDuelChallenger() const { return DuelChallenger; }
	AVGCharacterBase* GetDuelTarget() const { return DuelTarget; }
	
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Lobby")
	int32 MinimumPlayersNeeded = 2;
	
	
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = L"") override;
	virtual void PostLogin(APlayerController *NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void TransitionToPhase(TSubclassOf<class UVGPhaseBase> NextPhase);
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void PushPhase(TSubclassOf<class UVGPhaseBase> NewPhase);
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void PopPhase();
	
	void CheckWinCondition();
	void ResetGameStatus();

	void StartDuelPhase(AVGCharacterBase* Challenger, AVGCharacterBase* Target);
	
	void CheckAllPlayersReady();
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void AssignRolesAndStartGame();
	
	// 이벤트 중개소
	void OnPlayerDeath(AVGCharacterBase* Killer, AVGCharacterBase* Victim);
	void OnMissionCleared(int32 TimeReducedAmount);
	void SubmitVote(AVGPlayerState* Voter, int32 TargetIndex);
	UFUNCTION()
	void HandleMissionClear(float ReduceTime);

	//채팅 관련 함수 -입력메시지를 뿌려주는 역할 -김형백
	void ProcessChatMessage(const FString& SenderName, const FString& Message);
	
	// 플레이어간 상호작용 받을 함수
	virtual void RequestDuelPhase_Implementation(AVGCharacterBase* Challenger, AVGCharacterBase* Target) override;
};
