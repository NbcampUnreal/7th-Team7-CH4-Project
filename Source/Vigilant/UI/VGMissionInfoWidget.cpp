#include "VGMissionInfoWidget.h"
#include "Mission/Definitions/VGMissionBase.h"
#include "Components/TextBlock.h"
#include "Common/VGGameplayTags.h"

void UVGMissionInfoWidget::RegisterMission(AVGMissionBase* Mission)
{
	if (!Mission)
	{
		return;
	}
	
	MissionDescriptionText->SetText(FText::FromString(Mission->GetMissionDescription()));
	Mission->OnMissionStateChanged.AddDynamic(this, &UVGMissionInfoWidget::HandleMissionStateChanged);
}

void UVGMissionInfoWidget::HandleMissionStateChanged(int32 MissionID, FGameplayTag NewStateTag)
{
	FLinearColor TextColor;
	if (NewStateTag == VigilantMissionTags::MissionCompleted)
	{
		TextColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (NewStateTag == VigilantMissionTags::MissionActive)
	{
		TextColor = FLinearColor(0.8f, 0.8f, 0.2f, 1.0f);
	}
	else
	{
		TextColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	MissionDescriptionText->SetColorAndOpacity(FSlateColor(TextColor));
}
