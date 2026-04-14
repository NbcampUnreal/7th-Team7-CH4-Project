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
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vigilant|Time")
	float PhaseEndTime;
	
	// 미션 페이즈 결과에 따른 보스 스탯 배율
	UPROPERTY(ReplicatedUsing = OnRep_BossNerfRate, BlueprintReadOnly, Category = "Vigilant|Stats")
	float BossNerfRate;
	
	// UI에서 사용할 남은 시간 구하는 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vigilant|Time")
	float GetRemainingPhaseTime() const;
	
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
	
};
