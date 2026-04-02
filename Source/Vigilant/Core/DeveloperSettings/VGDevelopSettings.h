// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VGDevelopSettings.generated.h"

class UVGUIDataAsset;
/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Vigilant Settings"))
class VIGILANT_API UVGDevelopSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	//SoftPtr로 경로만 참조 - 사용시 Load 필요, 동기, 비동기 함수는 인터넷검색.. 아직못외움
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UVGUIDataAsset> UIDataAssetClass;
	
};
