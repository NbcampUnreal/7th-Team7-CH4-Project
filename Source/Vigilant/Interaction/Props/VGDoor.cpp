// Fill out your copyright notice in the Description page of Project Settings.


#include "VGDoor.h"
#include "Net/UnrealNetwork.h"


AVGDoor::AVGDoor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AVGDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVGDoor, bIsLeftOpen);
	DOREPLIFETIME(AVGDoor, bIsRightOpen);
}

void AVGDoor::OnRep_bIsLeftOpen()
{
	K2_UpdateLeftDoorVisual();
}

void AVGDoor::OnRep_bIsRightOpen()
{
	K2_UpdateRightDoorVisual();
}



