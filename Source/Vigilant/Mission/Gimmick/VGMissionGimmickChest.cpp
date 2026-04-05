// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMissionGimmickChest.h"


// Sets default values
AVGMissionGimmickChest::AVGMissionGimmickChest()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AVGMissionGimmickChest::CanInteractWith(AVGCharacterBase* Interactor) const
{
	return Super::CanInteractWith(Interactor);
}

void AVGMissionGimmickChest::OnInteractWith(AVGCharacterBase* Interactor)
{
	Super::OnInteractWith(Interactor);
}
