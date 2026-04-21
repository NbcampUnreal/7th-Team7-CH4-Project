#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Equipment/VGEquippableActor.h"
#include "VGMissionBase.generated.h"

class AVGCharacterBase;
class AVGMissionGimmickBase;
class AVGMissionItemBase;
class UVGMissionDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, int32, MissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionStateChanged, int32, MissionID, FGameplayTag, NewStateTag);

UCLASS(Abstract)
class VIGILANT_API AVGMissionBase : public AActor
{
	GENERATED_BODY()

public:
	AVGMissionBase();
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetCurrentStateTag() const {return CurrentStateTag;}
	
	UFUNCTION(BlueprintCallable)
	bool HasMissionTag(FGameplayTag Tag) const;
	
	// DataAsset Getter
	UFUNCTION(BlueprintPure)
	int32 GetMissionID() const;
	UFUNCTION(BlueprintPure)
	FGameplayTag GetMissionTypeTag() const;
	UFUNCTION(BlueprintPure)
	FString GetMissionDescription() const;
	UFUNCTION(BlueprintPure)
	float GetMissionTimeLimit() const;
	UFUNCTION(BlueprintPure)
	TSubclassOf<AVGEquippableActor> GetRewardItemClass() const;
	UFUNCTION(BlueprintPure)
	float GetMissionClearReduceTime() const;
	
	// 완료 처리 — Base에서 담당
	virtual void CompleteMission();
	void NotifyMissionCompleted();
	
	
	// 기여자 등록 — 각 Mission/Gimmick에서 상호작용 시 호출
	virtual void RegisterContributor(AVGCharacterBase* Character);
	// 기여자 해제 - 상호작용 시 Toggle 등으로 인해 비활성 되었을 경우
	virtual void UnregisterContributor(AVGCharacterBase* Character);
	
	// 미션 초기화 시 기여자 목록 제거
	void ClearContributers();
	
	// Gimmicck 상호작용 시 기여자 확인 및 등록/삭제 처리
	UFUNCTION()
	virtual void OnGimmickInteracted(AVGMissionGimmickBase* Gimmick, AActor* Interactor);
	
protected:
	virtual void BeginPlay() override;
	
	void SetMissionState(FGameplayTag NewStateTag);
	
	UFUNCTION()
	virtual void OnRep_CurrentStateTag();
	
	// 어떤 Gimmick이 변경되었는지 확인하기
	UFUNCTION()
	virtual void OnGimmickStateChanged(
		AVGMissionGimmickBase* Gimmick, FGameplayTag Tag);
	
	// 어떤 Item이 변경되었는지 확인하기
	UFUNCTION()
	virtual void OnItemStateChanged(
		AVGMissionItemBase* Item, FGameplayTag Tag);
	
	// 기믹 순서 셔플
	void ShuffleGimmickIndexes();
	
	// 자식이 override — 보상 아이템 클래스/위치 결정
	virtual void SpawnRewardItems();
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnMissionCompleted OnMissionCompleted;
	
	UPROPERTY(BlueprintAssignable)
	FOnMissionStateChanged OnMissionStateChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission|Reward")
	bool bSpawnRewardAtMission;
	
protected:
	// 에디터에서 이 미션에 사용될 기믹과 아이템을 직접 지정
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission|Objects")
	TArray<TObjectPtr<AVGMissionGimmickBase>> MissionGimmicks;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission|Objects")
	TArray<TObjectPtr<AVGMissionItemBase>> MissionItems;
	
	// For TimeAttack
	//UPROPERTY(EditDefaultsOnly, Category = "Mission|TimeAttack")
	//float TimeLimit = 30.f;
	
	FTimerHandle MissionTimerHandle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<UVGMissionDataAsset> MissionData;
	
	// bool 2개(bIsActivated, bIsCompleted) 대신 Tag 하나
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStateTag)
	FGameplayTag CurrentStateTag;
	
	// 복수 태그 관리용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|Tags")
	FGameplayTagContainer MissionTags;
	
	// 보상 지급 관련
	// 미션 완료에 기여한 플레이어 목록 — 서버 전용
	UPROPERTY()
	TArray<TWeakObjectPtr<AVGCharacterBase>> Contributors;

	// 마지막으로 기여한 플레이어 (주 보상 대상)
	UPROPERTY()
	TWeakObjectPtr<AVGCharacterBase> LastContributor;
};
