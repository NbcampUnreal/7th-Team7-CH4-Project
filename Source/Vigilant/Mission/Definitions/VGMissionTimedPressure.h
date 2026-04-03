#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionTimedPressure.generated.h"

UCLASS()
class VIGILANT_API AVGMissionTimedPressure : public AVGMissionBase
{
	GENERATED_BODY()

public:
	AVGMissionTimedPressure();

protected:
	virtual void OnGimmickStateChanged(
			AVGMissionGimmickBase* Gimmick, FGameplayTag Tag) override;
private:
	void StartTimer();
	void OnTimerExpired();
	bool AreAllPressuresActive() const;
};
