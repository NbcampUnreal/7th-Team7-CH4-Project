#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "VGMissionBase.generated.h"

class AVGMissionGimmickBase;
class AVGMissionItemBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, int32, MissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionStateChanged, int32, MissionID, FGameplayTag, NewStateTag);

UCLASS()
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
	
	UFUNCTION(BlueprintCallable)
	int32 GetMissionID() const;
	
	// 소속 기믹 또는 아이템의 조건 충족 시 호출
	UFUNCTION()
	virtual void OnConditionMet();
	
	
	virtual void CompleteMission();
	void NotifyMissionCompleted();
	
protected:
	void SetMissionState(FGameplayTag NewStateTag);
	
	UFUNCTION()
	virtual void OnRep_CurrentStateTag();
	
public:
	// 에디터에서 이 미션에 사용될 기믹과 아이템을 직접 지정
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission|Objects")
	TArray<TObjectPtr<AVGMissionGimmickBase>> MissionGimmicks;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission|Objects")
	TArray<TObjectPtr<AVGMissionItemBase>> MissionItems;
	
	UPROPERTY(BlueprintAssignable)
	FOnMissionCompleted OnMissionCompleted;
	
	UPROPERTY(BlueprintAssignable)
	FOnMissionStateChanged OnMissionStateChanged;

protected:
	// For TimeAttack
	UPROPERTY(EditDefaultOnlyeOnly, Category = "Mission|TimeAttack")
	float TimeLimit = 30.f;
	
	FTimerHandle MissionTimerHandle;
	
	
	// bool 2개(bIsActivated, bIsCompleted) 대신 Tag 하나
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStateTag)
	FGameplayTag CurrentStateTag;
	
	// 에디터에서 지정하는 미션 타입 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|Tags")
	FGameplayTag MissionTypeTag;
	
	// 복수 태그 관리용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|Tags")
	FGameplayTagContainer MissionTags;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission")
	int32 MissionID = -1;
};
