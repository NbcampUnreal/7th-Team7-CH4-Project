#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionPressureSequence.generated.h"

class AVGMissionGimmickPressure;

UCLASS()
class VIGILANT_API AVGMissionPressureSequence : public AVGMissionBase
{
	GENERATED_BODY()

public:
	AVGMissionPressureSequence();
	
protected:
	virtual void OnGimmickStateChanged(
		AVGMissionGimmickBase* Gimmick, FGameplayTag Tag) override;
private:
	bool CheckSequenceOrder(AVGMissionGimmickPressure* Pressure);
	
	int32 CurrentSequenceIndex = 0;
	bool bIsResetting = false;
};
