#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionTimedCombat.generated.h"

class AVGMissionSandbag;

UCLASS()
class VIGILANT_API AVGMissionTimedCombat : public AVGMissionBase
{
	GENERATED_BODY()

public:
	AVGMissionTimedCombat();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnSandbagHitted();

private:
	UFUNCTION()
	void OnSandbagDefeated(AVGCharacterBase* LastAttacker);

	bool AreAllSandbagsDefeated() const;
	
	void StartTimer();
	void OnTimerExpired();
	
public:
	// 에디터에서 이 미션에 사용할 샌드백 등록
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission|Combat")
	TArray<TObjectPtr<AVGMissionSandbag>> MissionSandbags;
	
	FTimerHandle SandbagTimerHandle;
};
