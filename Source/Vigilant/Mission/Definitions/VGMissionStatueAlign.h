#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionStatueAlign.generated.h"

UCLASS()
class VIGILANT_API AVGMissionStatueAlign : public AVGMissionBase
{
	GENERATED_BODY()

public:
	AVGMissionStatueAlign();

protected:
	UFUNCTION()
	virtual void OnGimmickStateChanged(
		AVGMissionGimmickBase* Gimmick, FGameplayTag Tag) override;

private:
	bool AreAllStatuesAligned() const;
};
