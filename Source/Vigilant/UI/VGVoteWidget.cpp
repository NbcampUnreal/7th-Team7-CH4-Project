// Fill out your copyright notice in the Description page of Project Settings.


#include "VGVoteWidget.h"

#include "VGChatMessage.h"
#include "VGVoteSlotWidget.h"
#include "Subsystem/VGUIManagerSubsystem.h"
#include "Components/EditableText.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void UVGVoteWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (VoteChatText)
	{
		VoteChatText->OnTextCommitted.AddUniqueDynamic(this, &UVGVoteWidget::OnTextCommitted);
	}
	//VoteChatText->SetIsEnabled(true); // 만약 엔터 눌러서 채팅창 활성화를 하고싶다면!
	
	//초상화 대입

	SetPortraitRenderTarget();
	
}

void UVGVoteWidget::ActiveChatText()
{
	VoteChatText->SetFocus();
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
			ActiveChatText();
		}
	}
}



void UVGVoteWidget::SetPortraitRenderTarget()
{
	UWorld* World = GetWorld();
	if (!World || !World->GetGameState()) return;
	
	if (PortraitWrapBox)
	{
		PortraitWrapBox->ClearChildren();
	}

	int32 SlotIndex = 0;
	for (APlayerState* PS : World->GetGameState()->PlayerArray)
	{
		if (APawn* PlayerPawn = PS->GetPawn())
		{
			if (USceneCaptureComponent2D* CaptureComp = PlayerPawn->FindComponentByClass<USceneCaptureComponent2D>())
			{
				UVGVoteSlotWidget* SlotWidget = CreateWidget<UVGVoteSlotWidget>(GetOwningPlayer(), VoteSlotClass);
				if (SlotWidget)
				{
					//초상화 설정은 여기서
					SlotWidget->SetupSlot(SlotIndex, CaptureComp);
					SlotWidget->OnVoteSlotClickedDelegate.AddDynamic(this, &UVGVoteWidget::ProcessVoteClick);

					if (GetOwningPlayer())
					{
						UWrapBoxSlot* SlotRemoteCon= PortraitWrapBox->AddChildToWrapBox(SlotWidget);
						//SlotRemoteCon-> 이 변수로 슬롯의 세부 옵션을 조절가능(길이, Pending 등)
						SlotRemoteCon->pen
					}
				}
			}
		}
		
		
	}
	

}

void UVGVoteWidget::ProcessVoteClick(int32 SlotIndex)
{
	//버튼이 눌렸을때 전달받은 인덱스로 짤 로직
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
