// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "VGUIManagerSubsystem.generated.h"

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
	
	public:
	UFUNCTION(BlueprintPure)
	UVGHUDWidget* GetCurrentHUDWidget() const {return CurrentHUDWidget;}
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	//표시 업데이트 실질적으로 여기(HUD전달)
	UFUNCTION()
	void OnStaminaUpdate(float NewValue, float MaxValue);
	UFUNCTION()
	void OnHealthUpdate(float NewValue, float MaxValue);
	// (이용호 작업) 투표 전달용 함수
	UFUNCTION()
	void RequestSubmitVote(int32 TargetIndex);
	
	UFUNCTION()
	void EquipSlotChanged(int32 SlotIndex);
	
	UFUNCTION()
	void RelayReadyEvent(bool bReady);
	
	FUIOnPlayerReadySignature OnPlayerReadySignature;
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
#pragma endregion 김형백
	//UIWidget -> UIManager로 텍스트 전달(이부분) -> PlayerController 에게 토스
	void RequsetSendChatMessage(const FString& Message);
	
	UPROPERTY(BlueprintAssignable, Category = "VGUIManager")
	FOnChatMessageRequestedSignature OnChatMessageRequested;
	// (이용호 추가)  PlayerController가 구독할 변수 추가
	UPROPERTY(BlueprintAssignable, Category = "Vigilant|Events")
	FOnVoteRequestedSignature OnVoteRequested;
	
	// 마지막 단계 
	void LoggingChatMessage(const FString& Message);
};
