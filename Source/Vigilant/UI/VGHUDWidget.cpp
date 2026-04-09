// Fill out your copyright notice in the Description page of Project Settings.


#include "VGHUDWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"

void UVGHUDWidget::UpdateStaminaUI(float NewValue, float MaxValue)
{
	if (MaxValue > 0.f)
	{
		StaminaBar->SetPercent(NewValue/MaxValue);
	}
}

void UVGHUDWidget::UpdateHealthUI(float NewValue, float MaxValue)
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
