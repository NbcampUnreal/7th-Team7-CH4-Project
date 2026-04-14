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
	FText NickName = InputNick->GetText();
	if (NickName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("이름을 입력해 주세요!"));
		// 나중에 위젯으로 띄울지도?
		return;
	}
	if (NickName.ToString().Len() >= 10)
	{
		UE_LOG(LogTemp, Warning, TEXT("이름은 10글자를 넘을 수 없습니다."));
		// 나중에 위젯으로 띄울지도?
		return;
	}
	AVGTitlePlayerController* PlayerController = GetOwningPlayer<AVGTitlePlayerController>();
	if (IsValid(PlayerController) == true)
	{
		FText ServerIP = ServerIPEditableText->GetText();
		PlayerController->JoinServer(ServerIP.ToString(), NickName.ToString());

		UE_LOG(LogTemp, Warning, TEXT("접속 시도!"));
	}
}

void UVGTitleWidget::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
