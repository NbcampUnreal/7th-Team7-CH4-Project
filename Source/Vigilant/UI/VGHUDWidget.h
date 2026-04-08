// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VGHUDWidget.generated.h"

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
	UFUNCTION(Category = "UI")
	void UpdateStaminaUI(float NewValue, float MaxValue);
    
	// 체력도 미리 구현
	UFUNCTION(Category = "UI")
	void UpdateHealthUI(float NewValue, float MaxValue);
	
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;
};
