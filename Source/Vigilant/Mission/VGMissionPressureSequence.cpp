#include "VGMissionPressureSequence.h"
#include "Gimmick/VGMissionGimmickPressure.h"

AVGMissionPressureSequence::AVGMissionPressureSequence()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CurrentSequenceIndex = 0;
}

bool AVGMissionPressureSequence::CheckMissionCondition(AActor* Reporter)
{
	AVGMissionGimmickPressure* Pressure =
		Cast<AVGMissionGimmickPressure>(Reporter);
        
	if (!Pressure) return false;
	return CheckSequenceOrder(Pressure);
}

void AVGMissionPressureSequence::OnMissionActivated()
{
	// 순서 초기화
	CurrentSequenceIndex = 0;
}

bool AVGMissionPressureSequence::CheckSequenceOrder(AActor* Reporter)
{
	
}
