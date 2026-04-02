// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "VGUIManagerSubsystem.generated.h"

/**
 * 
 */
class UUserWidget;
UCLASS()
class VIGILANT_API UVGUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	

	UPROPERTY(EditDefaultsOnly, Category = "HUDClass")
	TObjectPtr<UUserWidget> CurrentHUDWidget;
	public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	
	void ShowHUD();
	void HideHUD();
	
	void ShowPopup();
	void ClosePopup();
	
};
