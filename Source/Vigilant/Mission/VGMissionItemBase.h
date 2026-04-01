// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VGMissionItemBase.generated.h"

UCLASS()
class VIGILANT_API AVGMissionItemBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVGMissionItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
