// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMissionGimmickBase.h"


// Sets default values
AVGMissionGimmickBase::AVGMissionGimmickBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AVGMissionGimmickBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVGMissionGimmickBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

