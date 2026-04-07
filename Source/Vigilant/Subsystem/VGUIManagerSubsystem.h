// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "VGUIManagerSubsystem.generated.h"

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
	public:
	UFUNCTION(BlueprintPure)
	UVGHUDWidget* GetCurrentHUDWidget() const {return CurrentHUDWidget;}
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION()
	void OnStaminaUpdate(float NewValue, float MaxValue);
	UFUNCTION()
	void OnHealthUpdate(float NewValue, float MaxValue);
	
	void ShowHUD();
	void HideHUD();
	
	void ShowPopup();
	void ClosePopup();
	
};
