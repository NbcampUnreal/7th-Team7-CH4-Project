#include "Core/VGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Core/VGGameMode.h"
#include "Core/VGPlayerState.h"
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
		UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 레디!"))
		VGGameMode->CheckAllPlayersReady();
    }
}

void AVGPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);
	if (IsLocalPlayerController() && GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
	{
		UVGUIManagerSubsystem* UISubsystem = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>();
		UISubsystem->ShowHUD();
	}
}
