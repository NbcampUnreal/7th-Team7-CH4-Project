// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VGVoteWidget.generated.h"

class UVGVoteSlotWidget;
class UWrapBox;
class UButton;
class UImage;
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
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> PortraitWrapBox;
	

	//채팅 로그 클래스
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ChatMessageClass;
	void AddChatMessage(const FString& MessageLog);
	protected:
	//엔터키시 채팅보냄
	UFUNCTION()
	void OnTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);
	//투표부분
	UPROPERTY(EditAnywhere, Category = "WidgetSlotClass")
	TSubclassOf<UVGVoteSlotWidget> VoteSlotClass;
	
	
	UFUNCTION()
	void SetPortraitRenderTarget();
	UFUNCTION()
	void ProcessVoteClick(int32 SlotIndex);
};
