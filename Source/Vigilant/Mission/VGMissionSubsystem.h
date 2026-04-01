#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "VGMissionSubsystem.generated.h"

class AVGMissionBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllMissionCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnMissionStateChanged, int32, MissionID, FGameplayTag, NewStateTag);

UCLASS()
class VIGILANT_API UVGMissionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void RegisterMission(AVGMissionBase* Mission);
	
	UFUNCTION()
	void OnMissionCompleted(int32 MissionID);
	
	// 특정 타입의 미션만 필터링
	UFUNCTION(BlueprintCallable)
	TArray<AVGMissionBase*> GetMissionsByTag(FGameplayTag TypeTag) const;
	
	// ID로 미션 찾기
	UFUNCTION(BlueprintCallable)
	AVGMissionBase* GetMissionsByID(int32 MissionID) const;
	
	UFUNCTION(BlueprintCallable)
	TArray<AVGMissionBase*> GetAllMissions();
	
	
	// 특정 상태인 미션 개수 반환 - UI 미션 진행도에 사용
	UFUNCTION(BlueprintCallable)
	int32 GetMissionCountByState(FGameplayTag StateTag) const;
	
	UFUNCTION(BlueprintCallable)
	float GetMissionProgress() const;

public:	
	// 모든 미션 완료 시 브로드케스트 -> GameMode가 구독하여 페이즈 전환 트리거
	UPROPERTY(BlueprintAssignable)
	FOnAllMissionCompleted OnAllMissionCompleted;
	
	// UI 구독 -> 미션 상태 변경 시마다 HUD 갱신?
	UPROPERTY(BlueprintAssignable)
	FOnMissionStateChanged OnMissionStateChanged;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<AVGMissionBase>> RegisteredMissions;
	
	TSet<int32> CompletedMissions;
};
