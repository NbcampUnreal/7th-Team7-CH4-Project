// Fill out your copyright notice in the Description page of Project Settings.


#include "VGTitlePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AVGTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController() == false)
	{
		return;
	}
	if (TitleWidgetClass)
	{
		TitleWidgetInstance = CreateWidget<UUserWidget>(this, TitleWidgetClass);
		if (TitleWidgetInstance)
		{
			TitleWidgetInstance->AddToViewport();
			
			FInputModeUIOnly InputMode;
			
			InputMode.SetWidgetToFocus(TitleWidgetInstance->GetCachedWidget());
			SetInputMode(InputMode);
			
			bShowMouseCursor = true;
		}
	}
}

void AVGTitlePlayerController::JoinServer(const FString& InIPAddress)
{
	FName NextLevelName = FName(*InIPAddress);
	UGameplayStatics::OpenLevel(GetWorld(), NextLevelName, true);
}
