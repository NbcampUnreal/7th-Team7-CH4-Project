// Fill out your copyright notice in the Description page of Project Settings.


#include "VGVoteWidget.h"

#include "VGChatMessage.h"
#include "VGVoteSlotWidget.h"
#include "Subsystem/VGUIManagerSubsystem.h"
#include "Components/EditableText.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Core/VGPlayerState.h"// 디커플링포기

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

	
	UVGUIManagerSubsystem* UIManager = GetOwningLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>();


	
	for (APlayerState* PS : World->GetGameState()->PlayerArray)
	{
		AVGPlayerState* VGPlayerState = Cast<AVGPlayerState>(PS);
		if (!VGPlayerState)
		{
			continue;
		}
		if (APawn* PlayerPawn = PS->GetPawn())
		{
			if (USceneCaptureComponent2D* CaptureComp = PlayerPawn->FindComponentByClass<USceneCaptureComponent2D>())
			{
				UVGVoteSlotWidget* SlotWidget = CreateWidget<UVGVoteSlotWidget>(GetOwningPlayer(), VoteSlotClass);
				if (SlotWidget)
				{
					int32 RealPlayerIndex = VGPlayerState->EntryIndex;

					//초상화 설정은 여기서
					SlotWidget->SetupSlot(RealPlayerIndex, CaptureComp);
					
					if (UIManager && UIManager->CachedPlayerNames.Contains(RealPlayerIndex))
					{
						FString PlayerName = UIManager->CachedPlayerNames[RealPlayerIndex];
						SlotWidget->SetNickName(PlayerName);
					}
					else
					{
						// 혹시 캐시가 비어있다면 최후의 수단으로 PlayerState에서 직접 가져옴
						FString FallbackName = VGPlayerState->VGPlayerName;
						SlotWidget->SetNickName(FallbackName);
					}
					
					
					
					
					SlotWidget->OnVoteSlotClickedDelegate.AddDynamic(this, &UVGVoteWidget::ProcessVoteClick);

					if (GetOwningPlayer())
					{
						UWrapBoxSlot* SlotRemoteCon= PortraitWrapBox->AddChildToWrapBox(SlotWidget);
						//SlotRemoteCon-> 이 변수로 슬롯의 세부 옵션을 조절가능(길이, Pending 등)
						SlotRemoteCon->SetPadding(FMargin(20.0f));
						
					}
				}
			}
		}
		
		
	}
	

}

void UVGVoteWidget::ProcessVoteClick(int32 SlotIndex)
{
	if (UVGUIManagerSubsystem* UIManager = GetOwningLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
	{
		// 서브 시스템에 전송
		UIManager->RequestSubmitVote(SlotIndex);
		
		if (PortraitWrapBox)
		{
			PortraitWrapBox->SetIsEnabled(false);
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
