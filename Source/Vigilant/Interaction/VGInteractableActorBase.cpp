// Fill out your copyright notice in the Description page of Project Settings.


#include "VGInteractableActorBase.h"


AVGInteractableActorBase::AVGInteractableActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AVGInteractableActorBase::CanInteract_Implementation(AVGCharacterBase* Interactor) const
{
	// [Fix] Interactor null 체크 — 외부에서 nullptr 전달 시 하위 클래스 크래시 방지
	if (!Interactor)
	{
		return false;
	}
	return CanInteractWith(Interactor);
}

void AVGInteractableActorBase::OnInteract_Implementation(AVGCharacterBase* Interactor)
{
	if (!Interactor)
	{
		return;
	}
	OnInteractWith(Interactor);
}

bool AVGInteractableActorBase::CanInteractWith(AVGCharacterBase* Interactor) const
{
	return true;
}

void AVGInteractableActorBase::OnInteractWith(AVGCharacterBase* Interactor)
{

}
