#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionKeyUnlock.generated.h"

UCLASS()
class VIGILANT_API AVGMissionKeyUnlock : public AVGMissionBase
{
	GENERATED_BODY()

public:
	AVGMissionKeyUnlock();

protected:
	UFUNCTION()
	virtual void OnGimmickStateChanged(
		AVGMissionGimmickBase* Gimmick, FGameplayTag Tag) override;

private:
	bool AreAllChestsUnlocked() const;
};
