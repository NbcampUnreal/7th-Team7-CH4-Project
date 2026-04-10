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
		// 이름 입력안하면 레디 불가능
		if(VGPlayerState->VGPlayerName.IsEmpty()) return;
		
		VGPlayerState->bIsReady = bReady;
	}
	
	if (AVGGameMode* VGGameMode = Cast<AVGGameMode>(GetWorld()->GetAuthGameMode()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 레디!"))
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
