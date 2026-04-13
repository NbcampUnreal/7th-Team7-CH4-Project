// Fill out your copyright notice in the Description page of Project Settings.


#include "VGTitleWidget.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Title/VGTitlePlayerController.h"

void UVGTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (PlayButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &UVGTitleWidget::OnPlayButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UVGTitleWidget::OnExitButtonClicked);
	}
}

void UVGTitleWidget::OnPlayButtonClicked()
{
	AVGTitlePlayerController* PlayerController = GetOwningPlayer<AVGTitlePlayerController>();
	if (IsValid(PlayerController) == true)
	{
		FText ServerIP = ServerIPEditableText->GetText();
		PlayerController->JoinServer(ServerIP.ToString());

		UE_LOG(LogTemp, Warning, TEXT("접속 시도!"));
	}
}

void UVGTitleWidget::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
