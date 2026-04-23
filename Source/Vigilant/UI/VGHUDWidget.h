// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VGHUDWidget.generated.h"

class UTextBlock;
struct FGameplayTag;
class USizeBox;
class UOverlay;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyDelegate, bool, bReady);

class UButton;
class UImage;
class UProgressBar;
/**
 * 
 */
UCLASS()
class VIGILANT_API UVGHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	// 스태미나
public:
	
	virtual void NativeConstruct() override;
	
	
	UFUNCTION(Category = "UI|Equipment")
	void SetEquipIcon(int32 SlotIndex, UTexture2D* IconTexture);

	UFUNCTION(Category = "UI|Equipment")
	void ClearEquipIcon(int32 SlotIndex);
	
	UFUNCTION(Category = "UI|HiddenPocket")
	void SetHiddenPocketIcon(UTexture2D* IconTexture);

	UFUNCTION(Category = "UI|HiddenPocket")
	void ClearHiddenPocketIcon();
	
	UFUNCTION(Category = "UI")
	void UpdateStaminaUI(float NewValue, float MaxValue);
    
	// 체력도 미리 구현
	UFUNCTION(Category = "UI")
	void UpdateHealthUI(float NewValue, float MaxValue);
	
	UFUNCTION(Category = "UI")
	void UpdateMissionUI(float NewValue, float MaxValue);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> MissionProgress;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> ReadyOverlay;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Equip_Right;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Equip_Left;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Equip_Right_Frame;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Equip_Left_Frame;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BloodImage;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> TimerBarSize;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyNotification;
	
	UFUNCTION()
	void UpdateReadyPeople(int32 ReadyPeoPle, int32 TotalPeople);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> HiddenPocket;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HiddenPocketIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RoleText;
	//레디를 올려보낼 델리게이
	FOnReadyDelegate OnReadyDelegate;
	
	UFUNCTION()
	void ChangeSelectedEquipSlot(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "UI|Time")
	void SetPhaseTimeData(float InStartTime, float InEndTime, bool Init = false);
	
	void SetMissionBarContract(float NerfRate);
	
	// (이용호 추가) 막고라 페이즈 들어갈 때 프로그레스바 멈추기 용
	UFUNCTION(BlueprintCallable, Category = "UI|Time")
	void StopPhaseTimeData();
	
	UFUNCTION()
	void UpdateTimePerSecond();
	FTimerHandle PhaseTimerHandle;
	float TargetStartTime = -1.f;
	float TargetNewEndTime = -1.f;
	float TargetOldEndTime = -1.f;
	// (이용호 추가) 기존 페이즈의 길이 저장용
	float OriginalPhaseDuration = -1.0f;
	// 최초 1회만 저장하도록 하기위한 불변수
	bool bIsDurationSet = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "UI|BarSize")
	float BarSizeMultiplier = 3.0f;
	
	
	UFUNCTION()
	void PauseUpdateTimer();
	void ResumeUpdateTimer();

	UFUNCTION()
	void OnReadyButtonClicked();
	
	UFUNCTION()
	void DisplayRole(FGameplayTag RoleTag);
	
	UFUNCTION()
	void HideRoleText();
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> RoleAnim;
	
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
	TObjectPtr<UTexture2D> GlowingFrame;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
	TObjectPtr<UTexture2D> NormalFrame;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="HpEffect")
	float ShowBloodRatio;
	
private:
	FTimerHandle RoleTextTimerHandle;
	
};
