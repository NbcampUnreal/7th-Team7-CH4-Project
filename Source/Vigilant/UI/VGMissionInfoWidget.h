#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTags.h"
#include "VGMissionInfoWidget.generated.h"

class UTextBlock;
class AVGMissionBase;

UCLASS()
class VIGILANT_API UVGMissionInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void RegisterMission(AVGMissionBase* Mission);
	
	void HandleMissionStateChanged(int32 MissionID, FGameplayTag NewStateTag);
	
protected:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* MissionDescriptionText;
};
