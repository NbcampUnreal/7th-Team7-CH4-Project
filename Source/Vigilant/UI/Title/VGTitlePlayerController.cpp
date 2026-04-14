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

void AVGTitlePlayerController::JoinServer(const FString& InIPAddress, const FString& NickName)
{
	// FName NextLevelName = FName(*InIPAddress);
	// UGameplayStatics::OpenLevel(GetWorld(), NextLevelName, true);
	
	// ?Name=닉네임 형태로 Options 문자열 구성
	FString URL = FString::Printf(TEXT("%s?VGName=%s"), *InIPAddress, *NickName);
    
	// URL을 FName으로 변환하여 OpenLevel 호출
	UGameplayStatics::OpenLevel(GetWorld(), FName(*URL), true);
}

