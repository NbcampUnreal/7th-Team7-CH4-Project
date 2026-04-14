// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VGChatMessage.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class VIGILANT_API UVGChatMessage : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ChatMessageTextBlock;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FontsizeControll")
	float FontSize = 20;
	
	UFUNCTION()
	void SetChatMessage(const FString& Message);
	
};
