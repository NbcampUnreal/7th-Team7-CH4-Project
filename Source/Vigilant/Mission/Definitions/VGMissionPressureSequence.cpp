#include "VGMissionPressureSequence.h"
#include "Mission/Gimmick/VGMissionGimmickPressure.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"

AVGMissionPressureSequence::AVGMissionPressureSequence()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CurrentSequenceIndex = 0;
}

void AVGMissionPressureSequence::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
	if (Tag != VigilantMissionTags::GimmickActive)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[PressureSeq] %s Request CheckClear"), *Gimmick->GetName());
	
	AVGMissionGimmickPressure* Pressure =
		Cast<AVGMissionGimmickPressure>(Gimmick);
        
	if (!Pressure)
	{
		return;
	}
	
	if (CheckSequenceOrder(Pressure))
	{
		// [Fix] SetMissionState 직접 호출 → CompleteMission으로 통일 (타이머 정리 등 포함)
		CompleteMission();
	}
}

bool AVGMissionPressureSequence::CheckSequenceOrder(AVGMissionGimmickPressure* Pressure)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[PressureSeq] Reported index=%d | Expected=%d"),
		Pressure->GetGimmickIndex(), CurrentSequenceIndex);
	
	if (Pressure->GetGimmickIndex() != CurrentSequenceIndex)
	{
		// 순서 틀림 — 모든 발판 상태 초기화
		UE_LOG(LogTemp, Warning, TEXT("[PressureSeq] WRONG ORDER → Reset all"));
		CurrentSequenceIndex = 0;
		for (AVGMissionGimmickBase* Gimmick : MissionGimmicks)
		{
			// [Fix] null 체크 추가 — 에디터에서 배열 요소가 비어있을 수 있음
			if (Gimmick)
			{
				Gimmick->ResetGimmickState();
			}
		}
		return false;
	}
	
	CurrentSequenceIndex++;
	UE_LOG(LogTemp, Warning,
		TEXT("[PressureSeq] Correct! Progress=%d/%d"),
		CurrentSequenceIndex, MissionGimmicks.Num())
	
	// 등록된 모든 발판을 순서대로 밟았으면 완료
	return CurrentSequenceIndex >= MissionGimmicks.Num();
}
