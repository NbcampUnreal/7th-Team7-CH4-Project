// Fill out your copyright notice in the Description page of Project Settings.


#include "VGTitleWidget.h"

#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

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
	 /* PlayerController = GetOwningPlayer<ADXTitlePlayerController>();
	if (IsValid(PlayerController) == true)
	{
		FText ServerIP = ServerIPEditableText->GetText();
		PlayerController->JoinServer(ServerIP.ToString());
	}*/
}

void UVGTitleWidget::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
