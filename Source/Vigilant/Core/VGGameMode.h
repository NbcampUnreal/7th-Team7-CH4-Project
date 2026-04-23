#pragma once

#include "CoreMinimal.h"
#include "VGPlayerState.h"
#include "GameFramework/GameMode.h"
#include "Core/Interface/VGGameModeInterface.h"
#include "VGGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionTimeRemainingChanged, float, ElapsedTime , float, TotalTime);

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
	
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Phase")
	TSubclassOf<class UVGPhaseBase> InitialPhase;
	
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Phase")
	TSubclassOf<class UVGPhaseBase> DuelPhaseClass;
	
	UPROPERTY(Transient)
	AVGCharacterBase* DuelChallenger;
	
	UPROPERTY(Transient)
	AVGCharacterBase* DuelTarget;
	
	// 이미 게임이 시작되었는지 체크용
	bool bGameHasStarted = false;
	
	virtual void PreLogin(
		const FString& Options,
		const FString& Address,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	
	// 6개의 슬롯이 찼는지 아닌지 체크하는 용도
	bool bSlotOccupied[6] = { false, };
	
	// 게임 한 사이클이 종료되었을 때 호출될 함수
	void HandleMatchFinished();
	
	int32 AssignPlayerSlot(class AVGPlayerState* VGPlayerState);
	
	
	//김형백 캐릭터 랜덤 매시용 난수
	TArray<int32> RandomMeshNumber;
	//김형백 미션 시간 진행상황 업데이트용 타이머핸들
	FTimerHandle MissionUpdateTimerHandle;
	// 시네마틱 재생을 위해 투표 결과 확인 UI가 사라진 플레이어 숫자 카운트용
	int32 ReadyPlayerCountForCinematic = 0;
	
	// 엔딩 시네마틱 관련
	// 게임 종료 시네마틱을 다 본 플레이어 수
	int32 ReadyPlayerCountForGameEnd = 0;
	// 중복 종료 방지용 변수
	bool bIsMatchEnding = false;
	// 누군가 튕겼을 때를 대비한 15초 안전장치 타이머
	FTimerHandle GameEndFailSafeTimerHandle;
	
public:
	
	AVGGameMode();
	
	void ClearDuelParticipants();
	
	AVGCharacterBase* GetDuelChallenger() const { return DuelChallenger; }
	AVGCharacterBase* GetDuelTarget() const { return DuelTarget; }
	
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Lobby")
	int32 MinimumPlayersNeeded = 2;
	
	// 감옥 스폰포인트 저장
	UPROPERTY(Transient)
	TMap<int32, class APlayerStart*> CachedJailSpawns;
	
	
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
  
	//미션 시간변경을 전달하는 델리게이
	FOnMissionTimeRemainingChanged OnMissionTimeRemainingChanged;
	void OnMissionTimeUpdated();
	
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	virtual FString InitNewPlayer(
		APlayerController* NewPlayerController,
		const FUniqueNetIdRepl& UniqueId,
		const FString& Options,
		const FString& Portal = TEXT("")) override;
	virtual void Logout(AController* Exiting) override;
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void TransitionToPhase(TSubclassOf<class UVGPhaseBase> NextPhase);
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void PushPhase(TSubclassOf<class UVGPhaseBase> NewPhase);
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void PopPhase();
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void CheckWinCondition();

	void StartDuelPhase(AVGCharacterBase* Challenger, AVGCharacterBase* Target);
	
	void CheckAllPlayersReady();
	// 페이즈가 종료될 때 호출될 함수
	void NotifyPhaseCompleted(class UVGPhaseBase* CompletedPhase);
	// 플레이어 행동 가능 여부 확인용 함수
	virtual bool CanPlayerAttack_Implementation(class AVGCharacterBase* Attacker) override;
	virtual bool CanPlayerInteract_Implementation(class AVGCharacterBase* Interactor, AActor* Target) override;
	virtual bool CanPlayerTakeDamage_Implementation(AActor* DamageCauser, class AVGCharacterBase* Target) override;
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Phase")
	void AssignRolesAndStartGame();
	
	// 이벤트 중개소
	virtual void NotifyPlayerDeath_Implementation(class AVGCharacterBase* Killer, class AVGCharacterBase* Victim) override;
	void SubmitVote(AVGPlayerState* Voter, int32 TargetIndex);
	UFUNCTION()
	void HandleMissionClear(float ReduceTime);

	//채팅 관련 함수 -입력메시지를 뿌려주는 역할 -김형백
	void ProcessChatMessage(const FString& SenderName, const FString& Message);
	
	// 플레이어간 상호작용 받을 함수
	virtual void RequestDuelPhase_Implementation(AVGCharacterBase* Challenger, AVGCharacterBase* Target) override;
	
	// 투표 결과 UI 끝났는지 보고용 함수
	void ReportVoteUIFinished();
	
	// 컨트롤러가 호출할 보고 함수
	void ReportGameEndUIFinished();
    
	// 실제 서버트래블을 실행하는 함수
	void ExecuteGameEndSequence();
};
