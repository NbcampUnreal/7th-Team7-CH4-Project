// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "VGUIManagerSubsystem.generated.h"

struct FGameplayTag;
class UVGInteractionWidget;
//디커플링을 위한 방송용 델리게이트
//채팅 메시지 전달용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatMessageRequestedSignature, const FString&, Message);
// (이용호 추가) 투표 전달용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoteRequestedSignature, int32, TargetIndex);

//플레이어 컨트롤러에 레디 상태 전달할 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIOnPlayerReadySignature, bool, bReady);

class UVGPopupWidget;
class UVGVoteWidget;
class UVGHUDWidget;
/**
 * 
 */
class UUserWidget;
UCLASS()
class VIGILANT_API UVGUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	

	UPROPERTY(Transient)
	TObjectPtr<UVGHUDWidget> CurrentHUDWidget;
	UPROPERTY(Transient)
	TObjectPtr<UVGVoteWidget> CurrentVoteWidget;
	UPROPERTY(Transient)
	TObjectPtr<UVGPopupWidget> CurrentPopupWidget;
	UPROPERTY(Transient)
	TObjectPtr<UVGInteractionWidget> CurrentInteractWidget;
	public:
	UFUNCTION(BlueprintPure)
	UVGHUDWidget* GetCurrentHUDWidget() const {return CurrentHUDWidget;}
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	UFUNCTION()
	void TransferMissionTimeData(float StartTime, float EndTime, bool Init = false);
	
	// (이용호 추가) HUD의 정지 함수를 호출할 함수
	void StopMissionTimeData();
	
	
	void PauseUpdateTimerToHUD();
	void ResumeUpdateTimerToHUD();
	//표시 업데이트 실질적으로 여기(HUD전달)
	UFUNCTION()
	void OnStaminaUpdate(float NewValue, float MaxValue);
	UFUNCTION()
	void OnHealthUpdate(float NewValue, float MaxValue);
	// (이용호 추가) 투표 전달용 함수
	UFUNCTION()
	void RequestSubmitVote(int32 TargetIndex);
	
	UFUNCTION()
	void EquipSlotChanged(int32 SlotIndex);
	
	UFUNCTION()
	void RelayReadyEvent(bool bReady);
	
	UFUNCTION()
	void SetEquipIcon(int32 SlotIndex, UTexture2D* Icon);
	void ClearEquipIcon(int32 SlotIndex);
	
	UFUNCTION()
	void UpdatePlayerName(int32 PlayerIndex, const FString& PlayerName);
	
	FUIOnPlayerReadySignature OnPlayerReadySignature;
	
	TMap<int32, FString> CachedPlayerNames;
	
	UFUNCTION()
	void ShowRoleUI(FGameplayTag RoleTag);
#pragma region 위젯 생성과 표시, 숨김
	void CreateHUDWidget();
	void CreateVoteWidget();
	void CreatePopupWidget();
	
	void ShowHUD();
	void HideHUD();
	
	UFUNCTION(BlueprintCallable)
	void ShowVote();
	UFUNCTION(BlueprintCallable)
	void HideVote();
	
	void ShowPopup();
	void HidePopup();
	
	
	void ShowInteract(const FString& InfoText, const APlayerController* PlayerController, const FVector& TargetLocation);
	void HideInteract();
#pragma endregion 김형백
	//UIWidget -> UIManager로 텍스트 전달(이부분) -> PlayerController 에게 토스
	void RequsetSendChatMessage(const FString& Message);
	
	UPROPERTY(BlueprintAssignable, Category = "VGUIManager")
	FOnChatMessageRequestedSignature OnChatMessageRequested;
	// (이용호 추가)  PlayerController가 구독할 변수 추가
	UPROPERTY(BlueprintAssignable, Category = "Vigilant|Events")
	FOnVoteRequestedSignature OnVoteRequested;
	// (이용호 추가) 서버 트래블시 현재 있던 UI 전부 파괴
	void ClearAllWidgets();
	// (이용호 추가) 월드 초기화 감지용
	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues);
	
	// 마지막 단계 
	void LoggingChatMessage(const FString& Message);
};
