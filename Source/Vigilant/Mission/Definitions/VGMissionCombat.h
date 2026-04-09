// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionCombat.generated.h"

class AVGMissionSandbag;
class AVGCharacterBase;

UCLASS()
class VIGILANT_API AVGMissionCombat : public AVGMissionBase
{
	GENERATED_BODY()

public:
	AVGMissionCombat();

protected:
	virtual void BeginPlay() override;
	
private:
	UFUNCTION()
	void OnSandbagDefeated(AVGCharacterBase* LastAttacker);

	bool AreAllSandbagsDefeated() const;

public:
	// 에디터에서 이 미션에 사용할 샌드백 등록
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission|Combat")
	TArray<TObjectPtr<AVGMissionSandbag>> MissionSandbags;
};
