// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGMissionBase.h"
#include "VGMissionRotatingStatue.generated.h"

UCLASS()
class VIGILANT_API AVGMissionRotatingStatue : public AVGMissionBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVGMissionRotatingStatue();

protected:
	// [Fix] Base 클래스에서 이미 UFUNCTION() 선언됨 — override에 중복 지정 제거
	virtual void OnGimmickStateChanged(
		AVGMissionGimmickBase* Gimmick, FGameplayTag Tag) override;
	
	virtual void SpawnRewardItems() override;
private:
	bool AreAllStatueAligned() const;
};
