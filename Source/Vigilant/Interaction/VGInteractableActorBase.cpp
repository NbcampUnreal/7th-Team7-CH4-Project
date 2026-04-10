#include "VGInteractableActorBase.h"


AVGInteractableActorBase::AVGInteractableActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AVGInteractableActorBase::CanInteract_Implementation(AActor* Interactor) const
{
	return CanInteractWith(Interactor);
}

void AVGInteractableActorBase::OnInteract_Implementation(AActor* Interactor, const FTransform& InteractTransform)
{
	OnInteractWith(Interactor, InteractTransform);
}

bool AVGInteractableActorBase::CanInteractWith(AActor* Interactor) const
{
	return true;
}

void AVGInteractableActorBase::OnInteractWith(AActor* Interactor, const FTransform& InteractTransform)
{

}