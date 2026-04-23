#include "Core/VGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Core/VGGameMode.h"
#include "Core/VGGameState.h"
#include "Core/VGPlayerState.h"
#include "Common/VGGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/VGUIManagerSubsystem.h"
#include "TimerManager.h"
//#include "UI/VGHUDWidget.h"


AVGPlayerController::AVGPlayerController()
	: InputMappingContext(nullptr)
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
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	if (AVGGameState* VGGameState = GetWorld()->GetGameState<AVGGameState>())
	{
		VGGameState->OnPhaseChanged.AddUniqueDynamic(this, &AVGPlayerController::HandleUIByPhase);
	}
	else
	{
		// 게임스테이트 없는 상태면 있을 때까지 바인드 시도
		GetWorld()->GetTimerManager().SetTimer(BindTimerHandle, this, &AVGPlayerController::TryBindGameState, 0.1f,
		                                       true);
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* VGUIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
		{
			VGUIManager->OnVoteRequested.AddUniqueDynamic(this, &AVGPlayerController::Server_SubmitVote);
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

		//마우스 게임으로!
		FInputModeGameOnly InputGameOnly;

		SetInputMode(InputGameOnly);
		bShowMouseCursor = false;
		UE_LOG(LogTemp, Warning, TEXT("[VGGameMode] 서버: 모든 플레이어 준비 체크 시작"));
		Client_SetInputToGame();

		VGGameMode->CheckAllPlayersReady();
	}
}


void AVGPlayerController::Server_SetName_Implementation(const FString& NewName)
{
	if (AVGPlayerState* VGPlayerState = GetPlayerState<AVGPlayerState>())
	{
		VGPlayerState->VGPlayerName = NewName;
		UE_LOG(LogTemp, Warning, TEXT("[Server] %d번 플레이어 이름 설정: %s"), VGPlayerState->EntryIndex, *NewName);
	}
}

void AVGPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);

	if (IsLocalController())
	{
		if (AVGPlayerState* VGPlayerState = GetPawn()->GetPlayerState<AVGPlayerState>())
		{
			if (!VGPlayerState->bIsReady)
			{
				FInputModeGameAndUI InputMode;
				SetInputMode(InputMode);
				bShowMouseCursor = true;

				if (APawn* MyPawn = GetPawn())
				{
					MyPawn->DisableInput(this);
				}
			}
		}
		if (UVGUIManagerSubsystem* UIManager = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
		{
			UIManager->ShowHUD();

			UIManager->OnChatMessageRequested.AddUniqueDynamic(this, &AVGPlayerController::OnChatMessageReceived);
			UIManager->OnPlayerReadySignature.AddUniqueDynamic(this, &AVGPlayerController::Server_SetReady);

			// 있으면 바로 연결
			if (AVGGameState* VGGameState = GetWorld()->GetGameState<AVGGameState>())
			{
				float StartTime = VGGameState->PhaseStartTime;
				float EndTime = VGGameState->PhaseEndTime;
				UIManager->TransferMissionTimeData(StartTime, EndTime);

				VGGameState->OnPhaseChanged.AddUniqueDynamic(this, &AVGPlayerController::HandleUIByPhase);
				VGGameState->OnPhaseEndTimeChanged.AddUniqueDynamic(this, &AVGPlayerController::HandleTimeReduced);
			}
			else
			{
				GetWorld()->GetTimerManager().SetTimer(
					BindTimerHandle,
					this,
					&AVGPlayerController::TryBindGameState,
					0.1f,
					true
				);
				UE_LOG(LogTemp, Warning, TEXT("[AVGPlayerController] GameState 대기 중"));
			}
		}
	}
}

void AVGPlayerController::OnPossess(class APawn* P)
{
	//빙의하고 바인드 연결 스탯 컴뽀넌트랑 
	//서버에서만 실행되는 함수
	Super::OnPossess(P);
}

void AVGPlayerController::OnChatMessageReceived(const FString& Message)
{
	// UI매니저 델리게이트를 통해 Message 수신 후 RPC로 서버에 전달
	Server_SendChatMessage(Message);
}

void AVGPlayerController::Server_SubmitVote_Implementation(int32 TargetIndex)
{
	if (AVGGameMode* GameMode = Cast<AVGGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AVGPlayerState* VGPlayerState = GetPlayerState<AVGPlayerState>())
		{
			GameMode->SubmitVote(VGPlayerState, TargetIndex);
		}
	}
}

void AVGPlayerController::ReceiveChatMessage(const FString& Message)
{
	Client_ReceiveChatMessage(Message);
}

void AVGPlayerController::ShowInteractUI(const FString& InteractText, const FVector& TargetLocation, bool bShow)
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* VGUIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
		{
			if (bShow == true)
			{
				VGUIManager->ShowInteract(InteractText, this, TargetLocation);
		
				
			}
			else
			{
				VGUIManager->HideInteract();
			}
		}
	}
}

void AVGPlayerController::UpdateEquipIconUI(int32 SlotIndex, UTexture2D* Icon)
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
		{
			// UIManager를 거쳐 HUD의 SetEquipIcon 호출
			if (UIManager->GetCurrentHUDWidget())
			{
				UIManager->SetEquipIcon(SlotIndex, Icon);
			}
		}
	}
}

void AVGPlayerController::ClearEquipIconUI(int32 SlotIndex)
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
		// UIManager를 거쳐 HUD의 ClearEquipIcon 호출
		{
			if (UIManager->GetCurrentHUDWidget())
			{
				UIManager->ClearEquipIcon(SlotIndex);
			}
		}
	}
}

void AVGPlayerController::UpdatePlayerNameUI(int32 PlayerIndex, const FString& PlayerName)
{
	PlayerNameInController = PlayerName;
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
		{
			// UIManager로 데이터 전달
			UIManager->UpdatePlayerName(PlayerIndex, PlayerNameInController);
		}
	}
}

void AVGPlayerController::ShowRoleNotificationUI(FGameplayTag RoleTag)
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
		{
			UIManager->ShowRoleUI(RoleTag);
		}
	}
}

void AVGPlayerController::Client_SetInputToGame_Implementation()
{
	FInputModeGameOnly InputGameOnly;
	SetInputMode(InputGameOnly);
	bShowMouseCursor = false;

	if (APawn* MyPawn = GetPawn())
	{
		MyPawn->EnableInput(this);
	}

	UE_LOG(LogTemp, Warning, TEXT("[VGPlayerController] 클라이언트: 게임 모드로 전환 및 마우스 숨김"));
}

void AVGPlayerController::HandleUIByPhase(FGameplayTag NewPhaseTag)
{
	UVGUIManagerSubsystem* VGUIManager = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>();
	if (!VGUIManager) return;

	if (NewPhaseTag.MatchesTag(VigilantPhaseTags::PhaseMission))
	{
		if (AVGGameState* VGGameState = GetWorld()->GetGameState<AVGGameState>())
		{
			float StartTime = VGGameState->PhaseStartTime;
			float EndTime = VGGameState->PhaseEndTime;
			VGUIManager->TransferMissionTimeData(StartTime, EndTime, true);
		}
	}
	else
	{
		// 미션 페이즈에서 다른 페이즈 넘어가면 프로그레스바 게이지 채우기 중지
		VGUIManager->StopMissionTimeData();
	}

	// 투표 페이즈 태그가 있으면 UI 띄움
	if (NewPhaseTag.MatchesTag(VigilantPhaseTags::PhaseVote))
	{
		VGUIManager->ShowVote();
		VGUIManager->HideHUD();
	}
	// 투표 페이즈 아니면 닫음
	else
	{
		VGUIManager->HideVote();
		VGUIManager->ShowHUD();
	}
}

void AVGPlayerController::TryBindGameState()
{
	if (AVGGameState* VGGameState = GetWorld()->GetGameState<AVGGameState>())
	{
		// 델리게이트 연결
		VGGameState->OnPhaseChanged.AddUniqueDynamic(this, &AVGPlayerController::HandleUIByPhase);
		VGGameState->OnPhaseEndTimeChanged.AddUniqueDynamic(this, &AVGPlayerController::HandleTimeReduced);

		// UI 연동 안된 클라이언트도 연동
		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UVGUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
			{
				UIManager->TransferMissionTimeData(VGGameState->PhaseStartTime, VGGameState->PhaseEndTime);
			}
		}

		GetWorld()->GetTimerManager().ClearTimer(BindTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("[VGPlayerController] GameState 바인딩 성공"));
	}
}

void AVGPlayerController::Client_ReceiveChatMessage_Implementation(const FString& Message)
{
	if (IsLocalPlayerController() && GetLocalPlayer())
	{
		if (UVGUIManagerSubsystem* UIManager = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
		{
			UIManager->LoggingChatMessage(Message);
		}
	}
}

void AVGPlayerController::Server_SendChatMessage_Implementation(const FString& ChatText)
{
	FString SenderName = TEXT("Unknown");

	if (AVGPlayerState* VGPlayerState = GetPlayerState<AVGPlayerState>())
	{
		SenderName = VGPlayerState->VGPlayerName;
	}

	if (AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(this))
	{
		AVGGameMode* VGGameMode = Cast<AVGGameMode>(GameModeBase);
		if (VGGameMode)
		{
			VGGameMode->ProcessChatMessage(SenderName, ChatText);
		}
	}
}

void AVGPlayerController::HandleTimeReduced(float NewEndTime)
{
	if (UVGUIManagerSubsystem* VGUIManager = GetLocalPlayer()->GetSubsystem<UVGUIManagerSubsystem>())
	{
		if (AVGGameState* VGGameState = GetWorld()->GetGameState<AVGGameState>())
		{
			// 미션깨서 시간이 줄어들 때 UI 갱신
			VGUIManager->TransferMissionTimeData(VGGameState->PhaseStartTime, NewEndTime, false);
		}
	}
}

bool AVGPlayerController::Server_SendChatMessage_Validate(const FString& ChatText)
{
	return true;
}
