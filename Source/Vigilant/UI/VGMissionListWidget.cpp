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
		// 앞으로 등록될 미션을 위해 덷리게이트 바인딩
		MissionSubsystem->OnMissionRegistered.AddDynamic(this, &UVGMissionListWidget::AddMissionWidget);
		
		// 이미 등록된 미션들이 있다면 UI 생성
		const TArray<AVGMissionBase*>& Missions = MissionSubsystem->GetAllMissions();
		for (AVGMissionBase* Mission : Missions)
		{
			AddMissionWidget(Mission);
		}
	}
}

void UVGMissionListWidget::AddMissionWidget(AVGMissionBase* Mission)
{
	if (!Mission)
	{
		return;
	}
			
	UVGMissionInfoWidget* MissionInfoWidget 
		= CreateWidget<UVGMissionInfoWidget>(GetOwningPlayer(), MissionInfoWidgetClass);
	// MinsiInfoScroll에 추가하기
	if (!MissionInfoWidget)
	{
		return;
	}
			
	MissionInfoScrollBox->AddChild(MissionInfoWidget);
	MissionInfoWidget->RegisterMission(Mission);
}
