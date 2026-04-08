#include "Core/VGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Core/VGGameMode.h"
#include "Core/VGPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/VGUIManagerSubsystem.h"

AVGPlayerController::AVGPlayerController()
	:InputMappingContext(nullptr)
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
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
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
		VGGameMode->CheckAllPlayersReady();
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
		}
	}
}

void AVGPlayerController::OnChatMessageReceived(const FString& Message)
{
	// UI매니저 델리게이트를 통해 Message 수신 후 RPC로 서버에 전달
	Server_SendChatMessage(Message);
}

void AVGPlayerController::ReceiveChatMessage(const FString& Message)
{
	Client_ReceiveChatMessage(Message);
}

void AVGPlayerController::Client_ReceiveChatMessage_Implementation(const FString& Message)
{
	if (IsLocalPlayerController()&& GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* UIManager = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
		{
			UIManager->LoggingChatMessage(Message);
		}
	}
}

void AVGPlayerController::Server_SendChatMessage_Implementation(const FString& ChatText)
{
	//TODO : 플레이어 이름 가져오기
	if (AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(this))
	{
		AVGGameMode* GameMode = Cast<AVGGameMode>(GameModeBase);
		if (GameMode)
		{
			GameMode->ProcessChatMessage(FString(TEXT("임시 이름입니다 용호님 여기 나중에 플레이어 이름 받으면 넣어주세요 감사합니다.")),ChatText);
		}
	}
}

bool AVGPlayerController::Server_SendChatMessage_Validate(const FString& ChatText)
{
	return true;
}
