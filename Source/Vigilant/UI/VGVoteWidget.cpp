// Fill out your copyright notice in the Description page of Project Settings.


#include "VGVoteWidget.h"

#include "Components/EditableText.h"

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
			
		}
	}
}
