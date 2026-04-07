// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VGMissionListWidget.generated.h"

class UScrollBox;
class UVGMissionInfoWidget;
class AVGMissionBase;

UCLASS()
class VIGILANT_API UVGMissionListWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitializeWidget();
	
	void AddMissionWidget(AVGMissionBase* Mission);
protected:
	UPROPERTY(meta=(BindWidget))
	UScrollBox* MissionInfoScrollBox;
	
	UPROPERTY(EditDefaultsOnly, Category = "Mission")
	TSubclassOf<UVGMissionInfoWidget> MissionInfoWidgetClass;

};
