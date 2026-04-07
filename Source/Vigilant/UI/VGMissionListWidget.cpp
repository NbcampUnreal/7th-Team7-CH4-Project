#include "VGMissionListWidget.h"
#include "Components/ScrollBox.h"
#include "UI/VGMissionInfoWidget.h"
#include "Mission/VGMissionSubsystem.h"

void UVGMissionListWidget::InitializeWidget()
{
	UWorld* World = GetOwningLocalPlayer() ? 
		GetOwningLocalPlayer()->GetWorld() : nullptr;
	
	if (World == nullptr || !MissionInfoWidgetClass)
	{
		return;
	}
	
	if (UVGMissionSubsystem* MissionSubsystem = 
		World->GetSubsystem<UVGMissionSubsystem>())
	{
		const TArray<AVGMissionBase*>& Missions = MissionSubsystem->GetAllMissions();
		
		for (AVGMissionBase* Mission : Missions)
		{
			if (!Mission)
			{
				continue;
			}
			
			UVGMissionInfoWidget* MissionInfoWidget 
				= CreateWidget<UVGMissionInfoWidget>(GetOwningPlayer(), MissionInfoWidgetClass);
			// MinsiInfoScroll에 추가하기
			if (!MissionInfoWidget)
			{
				continue;
			}
			
			MissionInfoScrollBox->AddChild(MissionInfoWidget);
			MissionInfoWidget->RegisterMission(Mission);
		}
	}
}
