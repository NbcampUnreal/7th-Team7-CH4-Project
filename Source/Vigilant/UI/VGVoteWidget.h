// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VGVoteWidget.generated.h"

class UScrollBox;
class UEditableText;
/**
 * 
 */
UCLASS()
class VIGILANT_API UVGVoteWidget : public UUserWidget
{
	GENERATED_BODY()
	public:
	
	virtual void NativeConstruct() override;
	
	//엔터키시 채팅창 활성화
	UFUNCTION()
	void ActiveChatText();
	
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> VoteChatText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatScrollBox;
	
	protected:
	//엔터키시 채팅보냄
	UFUNCTION()
	void OnTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);
	
};
