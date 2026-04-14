// Fill out your copyright notice in the Description page of Project Settings.


#include "VGHUDWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

void UVGHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &UVGHUDWidget::OnReadyButtonClicked);
		
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

void UVGHUDWidget::OnReadyButtonClicked()
{
	OnReadyDelegate.Broadcast(true);
}
	

