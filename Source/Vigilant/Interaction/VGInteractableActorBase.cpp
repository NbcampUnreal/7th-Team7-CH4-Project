#include "VGInteractableActorBase.h"


AVGInteractableActorBase::AVGInteractableActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AVGInteractableActorBase::CanInteract_Implementation(AVGCharacterBase* Interactor) const
{
	return CanInteractWith(Interactor);
}

void AVGInteractableActorBase::OnInteract_Implementation(AVGCharacterBase* Interactor)
{
	OnInteractWith(Interactor);
}

bool AVGInteractableActorBase::CanInteractWith(AVGCharacterBase* Interactor) const
{
	return true;
}

void AVGInteractableActorBase::OnInteractWith(AVGCharacterBase* Interactor)
{

}

void AVGInteractableActorBase::PointOnInteract_Implementation(AVGCharacterBase* Interactor, const FHitResult& HitResult)
{
	PointOnInteract(Interactor, HitResult);
}

void AVGInteractableActorBase::PointOnInteractWith(AVGCharacterBase* Interactor, const FHitResult& HitResult)
{
	
}