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
	
	// [Fix] 인덴테이션 수정
	if (AVGGameMode* VGGameMode = Cast<AVGGameMode>(GetWorld()->GetAuthGameMode()))
	{
		VGGameMode->CheckAllPlayersReady();
	}
}

void AVGPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);
	// [Fix] GetLocalPlayer null 체크 추가 + 서브시스템 이중 조회 제거
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* UISubsystem = LP->GetSubsystem<UVGUIManagerSubsystem>())
		{
			UISubsystem->ShowHUD();
		}
	}
}
