#include "Core/VGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Core/VGGameMode.h"
#include "Core/VGGameState.h"
#include "Core/VGPlayerState.h"
#include "Common/VGGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/VGUIManagerSubsystem.h"
#include "TimerManager.h"
//#include "UI/VGHUDWidget.h"


AVGPlayerController::AVGPlayerController()
	: InputMappingContext(nullptr)
{
}



void AVGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}
	

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	if (AVGGameState* VGGameState = GetWorld()->GetGameState<AVGGameState>())
	{
		VGGameState->OnPhaseChanged.AddUniqueDynamic(this, &AVGPlayerController::HandleUIByPhase);
	}
	else
	{
		// 게임스테이트 없는 상태면 있을 때까지 바인드 시도
		GetWorld()->GetTimerManager().SetTimer(BindTimerHandle, this, &AVGPlayerController::TryBindGameState, 0.1f,
		                                       true);
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* VGUIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
		{
			VGUIManager->OnVoteRequested.AddUniqueDynamic(this, &AVGPlayerController::Server_SubmitVote);
		}
	}
}

void AVGPlayerController::Server_SetReady_Implementation(bool bReady)
{
	if (AVGPlayerState* VGPlayerState = GetPlayerState<AVGPlayerState>())
	{
		VGPlayerState->bIsReady = bReady;
	}
	if (AVGGameMode* VGGameMode = Cast<AVGGameMode>(GetWorld()->GetAuthGameMode()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 레디!"))

		//마우스 게임으로!
		FInputModeGameOnly InputGameOnly;

		SetInputMode(InputGameOnly);
		bShowMouseCursor = false;

		VGGameMode->CheckAllPlayersReady();
	}
}

void AVGPlayerController::Client_SetReady_Implementation(bool bReady)
{
	if (AVGGameMode* VGGameMode = Cast<AVGGameMode>(GetWorld()->GetAuthGameMode()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 클라이언트 레디!"))

		//마우스 게임으로!
		FInputModeGameOnly InputGameOnly;

		SetInputMode(InputGameOnly);
		bShowMouseCursor = false;

		VGGameMode->CheckAllPlayersReady();
	}
}

void AVGPlayerController::Server_SetName_Implementation(const FString& NewName)
{
	if (AVGPlayerState* VGPlayerState = GetPlayerState<AVGPlayerState>())
	{
		VGPlayerState->VGPlayerName = NewName;
		UE_LOG(LogTemp, Warning, TEXT("[Server] %d번 플레이어 이름 설정: %s"), VGPlayerState->EntryIndex, *NewName);
	}
}

void AVGPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);

	if (GetLocalPlayer())
	{
		if (IsLocalPlayerController() && GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
		{
			UVGUIManagerSubsystem* UISubsystem = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>();
			UISubsystem->ShowHUD();
		}

		if (UVGUIManagerSubsystem* UIManager = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
		{
			UIManager->OnChatMessageRequested.AddUniqueDynamic(this, &AVGPlayerController::OnChatMessageReceived);
			UIManager->OnPlayerReadySignature.AddUniqueDynamic(this, &AVGPlayerController::Server_SetReady);
			
			
			if (AVGGameState* VGGameState = GetWorld()->GetGameState<AVGGameState>())
			{
				float StartTime=VGGameState->PhaseStartTime;
				float EndTime = VGGameState->PhaseEndTime;
				UIManager->TransferMissionTimeData(StartTime, EndTime);
			}
			
		}
	}
}

void AVGPlayerController::OnPossess(class APawn* P)
{
	//빙의하고 바인드 연결 스탯 컴뽀넌트랑 
	//서버에서만 실행되는 함수
	Super::OnPossess(P);
}

void AVGPlayerController::OnChatMessageReceived(const FString& Message)
{
	// UI매니저 델리게이트를 통해 Message 수신 후 RPC로 서버에 전달
	Server_SendChatMessage(Message);
}

void AVGPlayerController::Server_SubmitVote_Implementation(int32 TargetIndex)
{
	if (AVGGameMode* GameMode = Cast<AVGGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AVGPlayerState* VGPlayerState = GetPlayerState<AVGPlayerState>())
		{
			GameMode->SubmitVote(VGPlayerState, TargetIndex);
		}
	}
}

void AVGPlayerController::ReceiveChatMessage(const FString& Message)
{
	Client_ReceiveChatMessage(Message);
}

void AVGPlayerController::HandleUIByPhase(FGameplayTag NewPhaseTag)
{
	UVGUIManagerSubsystem* VGUIManager = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>();
	if (!VGUIManager) return;

	if (NewPhaseTag.MatchesTag(VigilantPhaseTags::PhaseMission))
	{
		if (AVGGameState* VGGameState = GetWorld()->GetGameState<AVGGameState>())
		{
			float StartTime = VGGameState->PhaseStartTime;
			float EndTime = VGGameState->PhaseEndTime;
			VGUIManager->TransferMissionTimeData(StartTime, EndTime, true);
		}
	}
	
	// 투표 페이즈 태그가 있으면 UI 띄움
	if (NewPhaseTag.MatchesTag(VigilantPhaseTags::PhaseVote))
	{
		VGUIManager->ShowVote();
		VGUIManager->HideHUD();
	}
	// 투표 페이즈 아니면 닫음
	else
	{
		VGUIManager->HideVote();
		VGUIManager->ShowHUD();
	}
}

void AVGPlayerController::TryBindGameState()
{
	if (AVGGameState* VGGameState = GetWorld()->GetGameState<AVGGameState>())
	{
		// 델리게이트 연결
		VGGameState->OnPhaseChanged.AddUniqueDynamic(this, &AVGPlayerController::HandleUIByPhase);

		GetWorld()->GetTimerManager().ClearTimer(BindTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("[VGPlayerController] GameState 바인딩 성공"));
	}
}

void AVGPlayerController::Client_ReceiveChatMessage_Implementation(const FString& Message)
{
	if (IsLocalPlayerController() && GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* UIManager = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
		{
			UIManager->LoggingChatMessage(Message);
		}
	}
}

void AVGPlayerController::Server_SendChatMessage_Implementation(const FString& ChatText)
{
	FString SenderName = TEXT("Unknown");

	if (AVGPlayerState* VGPlayerState = GetPlayerState<AVGPlayerState>())
	{
		SenderName = VGPlayerState->VGPlayerName;
	}

	if (AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(this))
	{
		AVGGameMode* VGGameMode = Cast<AVGGameMode>(GameModeBase);
		if (VGGameMode)
		{
			VGGameMode->ProcessChatMessage(SenderName, ChatText);
		}
	}
}

bool AVGPlayerController::Server_SendChatMessage_Validate(const FString& ChatText)
{
	return true;
}
