#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionTimedAll.generated.h"

UCLASS()
class VIGILANT_API AVGMissionTimedAll : public AVGMissionBase
{
	GENERATED_BODY()

public:
	AVGMissionTimedAll();

protected:
	virtual void OnGimmickStateChanged(
			AVGMissionGimmickBase* Gimmick, FGameplayTag Tag) override;
private:
	void StartTimer();
	void OnTimerExpired();
	bool AreAllGimmicActive() const;
};
