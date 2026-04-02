#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionPressureSequence.generated.h"

UCLASS()
class VIGILANT_API AVGMissionPressureSequence : public AVGMissionBase
{
	GENERATED_BODY()

protected:
	virtual bool CheckMissionCondition(AActor* Reporter) override;

	virtual void OnMissionActivated() override;
	
	bool CheckSequenceOrder(AActor* Reporter);
	
	int32 CurrentSequenceIndex = 0;
};
