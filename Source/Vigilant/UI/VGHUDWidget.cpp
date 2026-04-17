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
				BloodImage->SetRenderOpacity(TargetAlpha);
			}
			else
			{
				BloodImage->SetRenderOpacity(0.f);
			}
		}
	}
}

void UVGHUDWidget::UpdateMissionUI(float NewValue, float MaxValue)
{
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
	if (Init == true)
	{
		TargetOldEndTime = TargetNewEndTime;
	}
	if (!FMath::IsNearlyEqual(TargetNewEndTime, InEndTime))
	{
		TargetOldEndTime = TargetNewEndTime;
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


void UVGHUDWidget::UpdateTimePerSecond()
{
	if (!GetWorld()) return;

	AGameStateBase* BaseGameState = GetWorld()->GetGameState();
	float CurrentTime = BaseGameState ? BaseGameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();

	float TotalTime = TargetNewEndTime - TargetStartTime;
	float ElapsedTime = CurrentTime - TargetStartTime;

	if (TotalTime > 0.f)
	{
		float MissionTimeRatio = FMath::Clamp(ElapsedTime / TotalTime, 0.0f, 1.0f);
		
		// 프로그레스 바 업데이트
		MissionProgress->SetPercent(MissionTimeRatio);

		float OffsetSizeRaito = TotalTime/TargetOldEndTime;
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TimerBarSize->Slot))
		{
			FMargin CurrentOffsets = CanvasSlot->GetOffsets();
			CurrentOffsets.Right = 360.0f * OffsetSizeRaito;
			CanvasSlot->SetOffsets(CurrentOffsets);
		}

		
		if (CurrentTime >= TargetNewEndTime)
		{
			GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
		}
	}
}

void UVGHUDWidget::OnReadyButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("버튼 누름"));
	OnReadyDelegate.Broadcast(true);
}
	

