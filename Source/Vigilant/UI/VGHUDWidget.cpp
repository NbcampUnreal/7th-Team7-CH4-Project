// Fill out your copyright notice in the Description page of Project Settings.


#include "VGHUDWidget.h"

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
}
