// Fill out your copyright notice in the Description page of Project Settings.


#include "VGVoteWidget.h"

#include "VGChatMessage.h"
#include "Subsystem/VGUIManagerSubsystem.h"
#include "Components/EditableText.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/ScrollBox.h"
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
	SetCharacterPortrait();
	
	
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

void UVGVoteWidget::SetCharacterPortrait()
{
	UWorld* World = GetWorld();
	if (!World || !World->GetGameState()) return;

	// 만약 에디터에서 RT 배열을 안 채워놨다면 튕기는 걸 방지
	if (PortraitRenderTarget.IsEmpty())
	{
		return;
	}

	int32 SlotIndex = 0; // 0번칸(1번 RT)부터 시작!

	// 게임에 접속한 "모든 플레이어(인스턴스)들"을 한 명씩 불러와서 줄을 세웁니다.
	for (APlayerState* PS : World->GetGameState()->PlayerArray)
	{
		// 그 플레이어가 조종하는 몸통(폰)을 가져옵니다.
		if (APawn* PlayerPawn = PS->GetPawn())
		{
			// 몸통에 붙어있는 카메라를 찾아냅니다.
			if (USceneCaptureComponent2D* CaptureComp = PlayerPawn->FindComponentByClass<USceneCaptureComponent2D>())
			{
				// 배열 범위 초과 에러 방지 (플레이어가 6명인데 RT가 5개일 경우 등)
				if (PortraitRenderTarget.IsValidIndex(SlotIndex))
				{
					// 첫 번째 유저의 카메라엔 RT_1을, 두 번째 유저에겐 RT_2를 물려줍니다.
					CaptureComp->TextureTarget = PortraitRenderTarget[SlotIndex];

					// 찰칵! 
					CaptureComp->CaptureScene();
				}
			}
		}

		SlotIndex++; // 다음 RT 칸으로 이동!

		// 인원이 넘어가면 그만~
		if (SlotIndex >= World->GetGameState()->PlayerArray.Num()) break;
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
