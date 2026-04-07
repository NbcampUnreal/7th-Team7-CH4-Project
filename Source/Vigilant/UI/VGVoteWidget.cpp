// Fill out your copyright notice in the Description page of Project Settings.


#include "VGVoteWidget.h"

#include "VGChatMessage.h"
#include "Subsystem/VGUIManagerSubsystem.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"

void UVGVoteWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (VoteChatText)
	{
		VoteChatText->OnTextCommitted.AddDynamic(this, &UVGVoteWidget::OnTextCommitted);
	}
	
	//VoteChatText->SetIsEnabled(false); // 만약 엔터 눌러서 채팅창 활성화를 하고싶다면!
	
}

void UVGVoteWidget::ActiveChatText()
{
}

void UVGVoteWidget::OnTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		if (VoteChatText)
		{
			if (UVGUIManagerSubsystem* UIManager = GetOwningLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
			{
				UIManager->RequsetSendChatMessage(InText.ToString());
			}
			VoteChatText->SetText(FText());
			
		}
	}
}

void UVGVoteWidget::AddChatMessage(const FString& MessageLog)
{
	if (GetOwningPlayer())
	{
		UVGChatMessage* ChatMessageWidget = CreateWidget<UVGChatMessage>(GetOwningPlayer(), ChatMessageClass);
		if (ChatMessageWidget)
		{
			ChatMessageWidget->SetChatMessage(MessageLog);
			ChatScrollBox->AddChild(ChatMessageWidget);
			ChatScrollBox->ScrollToEnd();
			ChatScrollBox->bAnimateWheelScrolling = true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("로컬플레이어없음"));
	}
}
