// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/VGInteractable.h"

// Add default functionality here for any IVGInteractable functions that are not pure virtual.

bool IVGInteractable::CanInteract_Implementation(AVGCharacterBase* Interactor) const
{
	return false; 
}

void IVGInteractable::OnInteract_Implementation(AVGCharacterBase* Interactor)
{
	
}
