// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "VGMissionGimmickPressure.generated.h"

UCLASS()
class VIGILANT_API AVGMissionGimmickPressure : public AVGMissionGimmickBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVGMissionGimmickPressure();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
