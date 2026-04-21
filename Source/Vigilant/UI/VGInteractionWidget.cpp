// Fill out your copyright notice in the Description page of Project Settings.


#include "VGInteractionWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UVGInteractionWidget::SetInteractText(const FString& InfoText)
{
	if (InteractText)
	{
		InteractText->SetText(FText::FromString(InfoText));
	}
}

void UVGInteractionWidget::HideInteract()
{
	TargetWorldLocation = FVector::ZeroVector;
	RemoveFromParent();
}

void UVGInteractionWidget::SetTargetWorldLocation(const FVector& InLocation)
{
	TargetWorldLocation = InLocation;
}

void UVGInteractionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	//실시간 위젯 위치 변경
	if (!TargetWorldLocation.IsNearlyZero())
	{
		if (APlayerController* PlayerController = GetOwningPlayer())
		{
			FVector2D ScreenPosition;
			
			
			if (UGameplayStatics::ProjectWorldToScreen(PlayerController, TargetWorldLocation, ScreenPosition))
			{
				SetPositionInViewport(ScreenPosition);
			}
		}
	}
}
