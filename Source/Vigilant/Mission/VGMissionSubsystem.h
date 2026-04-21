#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Mission/Definitions/VGMissionBase.h"
#include "VGMissionSubsystem.generated.h"

class AVGMissionBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllMissionCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionClearTimeReward, float, ReduceTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionRegistered, AVGMissionBase*, Mission);

UCLASS()
class VIGILANT_API UVGMissionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void Server_RegisterMission(AVGMissionBase* Mission);
	void Client_RegisterMission(AVGMissionBase* Mission);
	
	UFUNCTION(Server, Reliable)
	void Server_OnMissionCompleted(int32 MissionID);
	
	// 특정 타입의 미션만 필터링
	UFUNCTION(BlueprintCallable)
	TArray<AVGMissionBase*> GetMissionsByTag(FGameplayTag TypeTag) const;
	
	// ID로 미션 찾기
	// [Fix] 단일 객체 반환이므로 함수명 단수로 수정
	UFUNCTION(BlueprintCallable)
	AVGMissionBase* GetMissionByID(int32 MissionID) const;
	
	UFUNCTION(BlueprintCallable)
	const TArray<AVGMissionBase*>& GetAllMissions() const;
	
	
	// 특정 상태인 미션 개수 반환 - UI 미션 진행도에 사용
	UFUNCTION(BlueprintCallable)
	int32 GetMissionCountByState(FGameplayTag StateTag) const;
	
	UFUNCTION(BlueprintCallable)
	float GetMissionProgress() const;
	
protected:
	UFUNCTION()
	void HandleMissionStateChanged(int32 MissionID, FGameplayTag NewStateTag);
	
	bool IsServer() const;
public:	
	// 미션 등록 시 프로드케스트 -> Clinet에서 Widget에 미션 정보를 추가하는 트리거
	UPROPERTY(BlueprintAssignable)
	FOnMissionRegistered OnMissionRegistered;
	
	// 모든 미션 완료 시 브로드케스트 -> GameMode가 구독하여 페이즈 전환 트리거
	UPROPERTY(BlueprintAssignable)
	FOnAllMissionCompleted OnAllMissionCompleted;
	
	UPROPERTY(BlueprintAssignable)
	FOnMissionStateChanged OnMissionStateChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnMissionClearTimeReward OnMissionClearTimeReward;
private:
	UPROPERTY()
	TArray<TObjectPtr<AVGMissionBase>> RegisteredMissions;
	
	int32 CachedCompletedCount;
};
