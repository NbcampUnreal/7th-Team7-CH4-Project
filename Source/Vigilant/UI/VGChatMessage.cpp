// Fill out your copyright notice in the Description page of Project Settings.


#include "VGChatMessage.h"

#include "Components/TextBlock.h"

void UVGChatMessage::SetChatMessage(const FString& Message)
{
	if (ChatMessageTextBlock)
	{
		ChatMessageTextBlock->SetText(FText::FromString(Message));
		ChatMessageTextBlock->Font.Size = FontSize; //폰트 사이즈 조절용
	}
}
