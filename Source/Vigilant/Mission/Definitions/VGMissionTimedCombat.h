#pragma once

#include "CoreMinimal.h"
#include "VGMissionCombat.h"
#include "VGMissionTimedCombat.generated.h"

class AVGMissionSandbag;

UCLASS()
class VIGILANT_API AVGMissionTimedCombat : public AVGMissionCombat
{
	GENERATED_BODY()

public:
	AVGMissionTimedCombat();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnSandbagHitted();
	
	virtual void OnSandbagDefeated(AVGCharacterBase* LastAttacker) override;
private:
	
	void StartTimer();
	void OnTimerExpired();
};
