// teWidgetut your copyright notice in the Description page of Project Settings.


#include "VGUIManagerSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Data/VGUIDataAsset.h"
#include "Core/DeveloperSettings/VGDevelopSettings.h"
#include "UI/VGHUDWidget.h"
#include "UI/VGPopupWidget.h"
#include "UI/VGVoteWidget.h"
#include "GameFramework/PlayerController.h"

void UVGUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UVGUIManagerSubsystem::OnWorldInitialized);
}

void UVGUIManagerSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	
	Super::Deinitialize();
}

void UVGUIManagerSubsystem::TransferMissionTimeData(float StartTime, float EndTime, bool Init)
{
	if (CurrentHUDWidget)
	{
		CurrentHUDWidget->SetPhaseTimeData(StartTime, EndTime, Init);
	}
}

void UVGUIManagerSubsystem::StopMissionTimeData()
{
	if (CurrentHUDWidget)
	{
		CurrentHUDWidget->StopPhaseTimeData();
	}
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

void UVGUIManagerSubsystem::RequestSubmitVote(int32 TargetIndex)
{
	OnVoteRequested.Broadcast(TargetIndex);
}

void UVGUIManagerSubsystem::EquipSlotChanged(int32 SlotIndex)
{
	if (CurrentHUDWidget)
	{
		CurrentHUDWidget->ChangeSelectedEquipSlot(SlotIndex);
	}
}

void UVGUIManagerSubsystem::RelayReadyEvent(bool bReady)
{
	
	if (CurrentHUDWidget)
	{
		//함수 만들어서 가져와야하지만 일단 귀찮으니 직접참조
		CurrentHUDWidget->ReadyOverlay->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Warning, TEXT("UIManager 전달 성공"));
	}
	OnPlayerReadySignature.Broadcast(bReady);
}

void UVGUIManagerSubsystem::CreateHUDWidget()
{
	if (CurrentHUDWidget)
	{
		return;
	}
	//디벨롭세팅의 CDO를 가져온다
	const UVGDevelopSettings* UISettings = GetDefault<UVGDevelopSettings>();

	if (!UISettings->UIDataAssetClass.IsNull())
	{
		//
		UVGUIDataAsset* LoadedUIDataAsset = UISettings->UIDataAssetClass.LoadSynchronous();
		if (LoadedUIDataAsset && LoadedUIDataAsset->MainHUDWidgetClass)
		{
			CurrentHUDWidget = CreateWidget<UVGHUDWidget>(
				GetLocalPlayer()->GetPlayerController(GetWorld())
				, LoadedUIDataAsset->MainHUDWidgetClass
			);
			
			//HUD와 바인딩 델리게이트
			CurrentHUDWidget->OnReadyDelegate.AddUniqueDynamic(this, &UVGUIManagerSubsystem::RelayReadyEvent);
		}
	}
}

void UVGUIManagerSubsystem::CreateVoteWidget()
{
	//디벨롭세팅의 CDO를 가져온다
	const UVGDevelopSettings* UISettings = GetDefault<UVGDevelopSettings>();

	if (!UISettings->UIDataAssetClass.IsNull())
	{
		//
		UVGUIDataAsset* LoadedUIDataAsset = UISettings->UIDataAssetClass.LoadSynchronous();
		if (LoadedUIDataAsset && LoadedUIDataAsset->VoteWidgetClass)
		{
			CurrentVoteWidget = CreateWidget<UVGVoteWidget>(
				GetLocalPlayer()->GetPlayerController(GetWorld())
				, LoadedUIDataAsset->VoteWidgetClass
			);
		}
	}
}

void UVGUIManagerSubsystem::CreatePopupWidget()
{
	const UVGDevelopSettings* UISettings = GetDefault<UVGDevelopSettings>();

	if (!UISettings->UIDataAssetClass.IsNull())
	{
		//
		UVGUIDataAsset* LoadedUIDataAsset = UISettings->UIDataAssetClass.LoadSynchronous();
		if (LoadedUIDataAsset && LoadedUIDataAsset->PopupWidgetClass)
		{
			CurrentPopupWidget = CreateWidget<UVGPopupWidget>(
				GetLocalPlayer()->GetPlayerController(GetWorld())
				, LoadedUIDataAsset->PopupWidgetClass
			);
		}
	}
}

void UVGUIManagerSubsystem::ShowHUD()
{
	//생성이 안되어 있으면 생성, 
	if (!CurrentHUDWidget)
	{
		CreateHUDWidget();
	}

	//화면에 안띄워져 있으면 띄우기
	if (CurrentHUDWidget && !CurrentHUDWidget->IsInViewport())
	{
		CurrentHUDWidget->AddToViewport();
	}
}

void UVGUIManagerSubsystem::HideHUD()
{
	if (CurrentHUDWidget->IsInViewport())
	{
		CurrentHUDWidget->RemoveFromParent();
	}
}

void UVGUIManagerSubsystem::ShowVote()
{
	//생성이 안되어 있으면 생성, 
	if (!CurrentVoteWidget)
	{
		CreateVoteWidget();
	}

	//화면에 안띄워져 있으면 띄우기
	if (CurrentVoteWidget && !CurrentVoteWidget->IsInViewport())
	{
		CurrentVoteWidget->AddToViewport();
	}
	
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		// 1. 입력 모드를 UI 전용으로 설정
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(CurrentVoteWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        
		PC->SetInputMode(InputMode);

		// 2. 마우스 커서 활성화
		PC->bShowMouseCursor = true;
	}
}

void UVGUIManagerSubsystem::HideVote()
{
	if (IsValid(CurrentVoteWidget) && CurrentVoteWidget->IsInViewport())
	{
		CurrentVoteWidget->RemoveFromParent();
		
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			// 3. 입력 모드를 다시 게임 전용으로 복구
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);

			// 4. 마우스 커서 비활성화
			PC->bShowMouseCursor = false;
		}
	}
	
}

void UVGUIManagerSubsystem::ShowPopup()
{
	if (!CurrentPopupWidget)
	{
		CreatePopupWidget();
	}

	if (CurrentPopupWidget && !CurrentPopupWidget->IsInViewport())
	{
		CurrentPopupWidget->AddToViewport();
	}
}

void UVGUIManagerSubsystem::HidePopup()
{
	if (CurrentPopupWidget->IsInViewport())
	{
		CurrentPopupWidget->RemoveFromParent();
	}
}

void UVGUIManagerSubsystem::RequsetSendChatMessage(const FString& Message)
{
	OnChatMessageRequested.Broadcast(Message);
}

void UVGUIManagerSubsystem::ClearAllWidgets()
{
	if (CurrentHUDWidget)
	{
		CurrentHUDWidget->RemoveFromParent();
		CurrentHUDWidget = nullptr;
	}
	
	if (CurrentVoteWidget)
	{
		CurrentVoteWidget->RemoveFromParent();
		CurrentVoteWidget = nullptr;
	}
	
	if (CurrentPopupWidget)
	{
		CurrentPopupWidget->RemoveFromParent();
		CurrentPopupWidget = nullptr;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[VGUIManagerSubsystem] UI 전부 파괴"));
}

void UVGUIManagerSubsystem::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues)
{
	ClearAllWidgets();
}

void UVGUIManagerSubsystem::LoggingChatMessage(const FString& Message)
{
	if (CurrentVoteWidget)
	{
		CurrentVoteWidget->AddChatMessage(Message);
	}
}
