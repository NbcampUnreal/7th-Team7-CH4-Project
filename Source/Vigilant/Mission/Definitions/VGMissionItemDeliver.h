#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionItemDeliver.generated.h"

UCLASS()
class VIGILANT_API AVGMissionItemDeliver : public AVGMissionBase
{
	GENERATED_BODY()

public:
	AVGMissionItemDeliver();

protected:
	UFUNCTION()
	virtual void OnGimmickStateChanged(
		AVGMissionGimmickBase* Gimmick, FGameplayTag Tag) override;

private:
	bool AreAllAltarsPlaced() const;
};
