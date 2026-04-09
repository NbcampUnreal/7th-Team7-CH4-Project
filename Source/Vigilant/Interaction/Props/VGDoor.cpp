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
	DOREPLIFETIME(AVGDoor, bIsOpen);
}

void AVGDoor::OnInteractWith(AActor* Interactor, const FTransform& InteractTransform)
{
	if (!HasAuthority())
	{
		return;
	}
    
	OnRep_IsOpen();
}

void AVGDoor::OnRep_IsOpen()
{
	K2_OnDoorStateChanged(bIsOpen);
}



