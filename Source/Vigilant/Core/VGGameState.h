#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/GameState.h"
#include "GameplayTagContainer.h"
#include "VGGameState.generated.h"


// 후에 UI에서 사용할 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, FGameplayTag, NewPhaseTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionProgressUpdated, float, CurrentProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDuelWinnerAnnounced, const FString&, WinnerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossNerfUpdated, float, BossRate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhaseTimeChanged);
// 시네마틱 재생용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoteResultCinematic, int32, TargetEntryIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEndCinematic, FGameplayTag, WinnerTeamTag);

UCLASS()
class VIGILANT_API AVGGameState : public AGameState, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:
	AVGGameState();
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	

	// UI가 바인드할 채널
	UPROPERTY(BlueprintAssignable, Category = "Vigilant|Events")
	FOnPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Vigilant|Events")
	FOnMissionProgressUpdated OnMissionProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Vigilant|Events")
	FOnDuelWinnerAnnounced OnDuelWinnerAnnounced;
	
	UPROPERTY(BlueprintAssignable, Category = "Vigilant|Events")
	FOnBossNerfUpdated OnBossNerfUpdated;
	
	UPROPERTY(BlueprintAssignable, Category = "Vigilant|Events")
	FOnPhaseTimeChanged OnPhaseTimeChanged;
	
	// 서버가 갱신하고 클라이언트로 동기화되는 데이터
	
	// 현재 페이즈 상태
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPhaseTag, BlueprintReadOnly, Category = "Vigilant|Tag")
	FGameplayTag CurrentPhaseTag;

	// 미션 달성도 (0.0 ~ 1.0)
	UPROPERTY(ReplicatedUsing = OnRep_MissionProgress, BlueprintReadOnly, Category = "Vigilant|Events")
	float TotalMissionProgress;

	// 막고라 승리자 이름 (일회성 알림용)
	UPROPERTY(ReplicatedUsing = OnRep_DuelWinner, Transient)
	FString LastDuelWinnerName;

	// 페이즈가 끝나는 서버 시간
	UPROPERTY(ReplicatedUsing = OnRep_PhaseEndTime, BlueprintReadOnly, Category = "Vigilant|Time")
	float PhaseEndTime;
	
	UPROPERTY(ReplicatedUsing = OnRep_PhaseStartTime, BlueprintReadOnly, Category = "Vigilant|Time")
	float PhaseStartTime;
	
	// 미션 페이즈 결과에 따른 보스 스탯 배율
	UPROPERTY(ReplicatedUsing = OnRep_BossNerfRate, BlueprintReadOnly, Category = "Vigilant|Stats")
	float BossNerfRate;
	
	// 레디 완료한 플레이어 수 가져오는 함수
	UFUNCTION(BlueprintPure, Category = "Vigilant|Lobby")
	int32 GetReadyPlayerCount() const;

	// 현재 접속중인 총 플레이어 수 가져오는 함수
	UFUNCTION(BlueprintPure, Category = "Vigilant|Lobby")
	int32 GetTotalPlayerCount() const;
	
	// 투표 관련
	// 최다 득표자의 이름
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vigilant|VoteResult")
	FString VotedPlayerName;
	// 최다 득표자의 인덱스
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vigilant|VoteResult")
	int32 VotedPlayerIndex;
	// 동점 여부
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vigilant|VoteResult")
	bool bIsVoteTie;
	// 블루프린트 바인딩용
	UPROPERTY(BlueprintAssignable, Category = "Vigilant|Events")
	FOnVoteResultCinematic OnVoteResultCinematic;
	// 서버가 모든 클라이언트에서 시네마틱 델리게이트를 호출하도록 하는 RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayVoteResultCinematic(int32 TargetEntryIndex);
	
	// 엔딩 관련
	// 최종 승리 팀의 태그
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vigilant|Result")
	FGameplayTag WinnerTeamTag;
	// 블루프린트에서 바인딩용
	UPROPERTY(BlueprintAssignable, Category = "Vigilant|Events")
	FOnGameEndCinematic OnGameEndCinematic;
	// 서버가 클라이언트들에게 엔딩 시네마틱 재생을 지시하는 RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayGameEndCinematic(FGameplayTag InWinnerTeamTag);
	
	// UI에서 사용할 남은 시간 구하는 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vigilant|Time")
	float GetRemainingPhaseTime() const;
	
	// 현재까지 경과된 시간 계산
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Time")
	float GetElapsedTime() const;
	
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Tag")
    void SetCurrentPhaseTag(FGameplayTag NewTag);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 클라이언트 수신 알림 함수
	
	UFUNCTION()
	void OnRep_CurrentPhaseTag();

	UFUNCTION()
	void OnRep_MissionProgress();

	UFUNCTION()
	void OnRep_DuelWinner();
	
	UFUNCTION()
	void OnRep_BossNerfRate();
	
	UFUNCTION()
	void OnRep_PhaseStartTime();
	
	UFUNCTION()
	void OnRep_PhaseEndTime(float OldEndTime);
	
};
