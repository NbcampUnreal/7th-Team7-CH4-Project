// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VGTitleWidget.generated.h"

class UEditableText;
class UButton;
/**
 * 
 */
UCLASS()
class VIGILANT_API UVGTitleWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnPlayButtonClicked();
	UFUNCTION()
	void OnExitButtonClicked();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ServerIPEditableText;
};
