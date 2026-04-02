#include "Core/VGPlayerController.h"
#include "EnhancedInputSubsystems.h"
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

void AVGPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);
	if (IsLocalPlayerController() && GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
	{
		UVGUIManagerSubsystem* UISubsystem = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>();
		UISubsystem->ShowHUD();
	}
}
