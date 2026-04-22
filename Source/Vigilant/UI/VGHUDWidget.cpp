// Fill out your copyright notice in the Description page of Project Settings.


#include "VGHUDWidget.h"

#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "GameFramework/GameStateBase.h"

void UVGHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddUniqueDynamic(this, &UVGHUDWidget::OnReadyButtonClicked);
		
	}
}

void UVGHUDWidget::SetEquipIcon(int32 SlotIndex, UTexture2D* IconTexture)
{
	UImage* TargetImage = (SlotIndex == 1) ? Equip_Left : Equip_Right; // 왼손 오른손 판단
	if (TargetImage && IconTexture)
	{
		TargetImage->SetBrushFromTexture(IconTexture);
		TargetImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f)); // 투명도 복구
	}
}

void UVGHUDWidget::ClearEquipIcon(int32 SlotIndex)
{
	UImage* TargetImage = (SlotIndex == 1) ? Equip_Left : Equip_Right;
	if (TargetImage)
	{
		// 이미지를 투명하게 만들어 숨김
		TargetImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f));
	}
}

void UVGHUDWidget::UpdateStaminaUI(float NewValue, float MaxValue)
{
	if (MaxValue > 0.f)
	{
		StaminaBar->SetPercent(NewValue/MaxValue);
	}
}

void UVGHUDWidget::UpdateHealthUI(float NewValue, float MaxValue)
{
	if (BloodImage)
	{
		if (MaxValue > 0.f)
		{
			float HealthRatio = NewValue/MaxValue;
			float TargetAlpha = 0.5f - HealthRatio*0.5f;

			// 체력 20% 부터 표시됨
			if (NewValue/MaxValue <= 0.2f)
			{
				BloodImage->SetVisibility(ESlateVisibility::Visible);
				BloodImage->SetRenderOpacity(TargetAlpha);
			}
			else
			{
				BloodImage->SetVisibility(ESlateVisibility::Collapsed);
				BloodImage->SetRenderOpacity(0.f);
			}
		}
	}
}

void UVGHUDWidget::UpdateMissionUI(float NewValue, float MaxValue)
{
	// 보스 체력으로 퍼센트 적용
	if (MissionProgress && MaxValue > 0.0f)
	{
		float HealthRatio = FMath::Clamp(NewValue / MaxValue, 0.0f, 1.0f);
		MissionProgress->SetPercent(HealthRatio);
	}
}

void UVGHUDWidget::ChangeSelectedEquipSlot(int32 SlotIndex)
{
	if (GlowingFrame &&NormalFrame)
	{
		if (SlotIndex == 1) // 왼손 선택 상태면
		{
			Equip_Left_Frame->SetBrushFromTexture(GlowingFrame);
			Equip_Right_Frame->SetBrushFromTexture(NormalFrame);
		}
		else if (SlotIndex == 2) //오른손 선택 상태면
		{
			Equip_Right_Frame->SetBrushFromTexture(GlowingFrame);
			Equip_Left_Frame->SetBrushFromTexture(NormalFrame);
		}
		else
		{
			// 미래에 마피아 3번째 슬롯이 생긴다면 여기에 작성	
		}
	}
}
void UVGHUDWidget::SetPhaseTimeData(float InStartTime, float InEndTime, bool Init)
{
	TargetStartTime = InStartTime;
	
	float CalculatedDuration = InEndTime - InStartTime;
	if (!bIsDurationSet && CalculatedDuration > 1.0f)
	{
		OriginalPhaseDuration = CalculatedDuration; 
		bIsDurationSet = true;
	}
	
	// Init 값이 true 거나 들어온 EndTime 이 0.1f(막고라 페이즈같이 시간 제한이 없는 페이즈) 이면 초기화
	if (Init == true || TargetNewEndTime < 0.0f)
	{
		TargetNewEndTime = InEndTime;
	}
	
	TargetNewEndTime = InEndTime;
	
	
	if (GetWorld() && TargetNewEndTime > TargetStartTime)
	{
		
		GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
		
		
		GetWorld()->GetTimerManager().SetTimer(
			PhaseTimerHandle, 
			this, 
			&UVGHUDWidget::UpdateTimePerSecond, 
			0.5f, 
			true
		);
		
		// 타이머 시작과 동시에 0초 차 지연 없이 UI를 즉시 1회 반영
		UpdateTimePerSecond();
	}
}

void UVGHUDWidget::SetMissionBarContract(float NerfRate)
{
	if (!TimerBarSize || NerfRate <= 0.0f) return;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TimerBarSize->Slot))
	{
		float OffsetSizeRatio = 2.0f - NerfRate; 
		float NewOffsetSize = 360.0f * OffsetSizeRatio;
        
		FMargin CurrentOffsets = CanvasSlot->GetOffsets();
		CurrentOffsets.Right = NewOffsetSize; 
		CanvasSlot->SetOffsets(CurrentOffsets);
		
	}
}

void UVGHUDWidget::StopPhaseTimeData()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
	}
}


void UVGHUDWidget::UpdateTimePerSecond()
{
	if (!GetWorld() || !MissionProgress || !TimerBarSize) return;

	AGameStateBase* BaseGameState = GetWorld()->GetGameState();
	float CurrentTime = BaseGameState ? BaseGameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();

	float TotalTime = TargetNewEndTime - TargetStartTime;
	float ElapsedTime = CurrentTime - TargetStartTime;

	if (TotalTime > 0.f)
	{
		float MissionTimeRatio = FMath::Clamp(ElapsedTime / TotalTime, 0.0f, 1.0f);
		
		// 프로그레스 바 업데이트
		MissionProgress->SetPercent(MissionTimeRatio);

		float CurrentNerfRate = TotalTime / OriginalPhaseDuration; 
		float OffsetSizeRatio = 2.0f - CurrentNerfRate;
		
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TimerBarSize->Slot))
		{
			FMargin CurrentOffsets = CanvasSlot->GetOffsets();
			CurrentOffsets.Right = 360.0f * OffsetSizeRatio;
			CanvasSlot->SetOffsets(CurrentOffsets);
		}

		
		if (CurrentTime >= TargetNewEndTime)
		{
			GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
		}
	}
}

void UVGHUDWidget::PauseUpdateTimer()
{
	GetWorld()->GetTimerManager().PauseTimer(PhaseTimerHandle);
	UE_LOG(LogTemp, Warning, TEXT("[HUD] 미션 페이즈 타이머 일시정지"));

	
}

void UVGHUDWidget::ResumeUpdateTimer()
{
	GetWorld()->GetTimerManager().UnPauseTimer(PhaseTimerHandle);
	UE_LOG(LogTemp, Warning, TEXT("[HUD] 미션 페이즈 타이머 재개"));
}

void UVGHUDWidget::OnReadyButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("버튼 누름"));
	OnReadyDelegate.Broadcast(true);
}
	

