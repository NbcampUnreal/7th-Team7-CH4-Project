// Fill out your copyright notice in the Description page of Project Settings.


#include "VGUIManagerSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Data/VGUIDataAsset.h"
#include "Core/DeveloperSettings/VGDevelopSettings.h"
#include "UI/VGHUDWidget.h"



void UVGUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UVGUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UVGUIManagerSubsystem::OnStaminaUpdate(float NewValue, float MaxValue)
{
	if (CurrentHUDWidget)
	{
		CurrentHUDWidget->UpdateStaminaUI(NewValue, MaxValue);
	}
}

void UVGUIManagerSubsystem::OnHealthUpdate(float NewValue, float MaxValue)
{
	if (CurrentHUDWidget)
	{
		CurrentHUDWidget->UpdateHealthUI(NewValue, MaxValue);
	}
}

void UVGUIManagerSubsystem::ShowHUD()
{
	//디벨롭세팅의 CDO를 가져온다
	const UVGDevelopSettings* UISettings = GetDefault<UVGDevelopSettings>();
	
	if (!UISettings->UIDataAssetClass.IsNull())
	{
		UVGUIDataAsset* LoadedUIDataAsset = UISettings->UIDataAssetClass.LoadSynchronous();
		if (LoadedUIDataAsset && LoadedUIDataAsset->MainHUDWidgetClass)
		{
			CurrentHUDWidget = CreateWidget<UVGHUDWidget>(
				GetLocalPlayer()->GetPlayerController(GetWorld())
				,LoadedUIDataAsset->MainHUDWidgetClass
				);
			CurrentHUDWidget->AddToViewport();
		}

	}
	
}

void UVGUIManagerSubsystem::HideHUD()
{
}

void UVGUIManagerSubsystem::ShowPopup()
{
}

void UVGUIManagerSubsystem::ClosePopup()
{
}
