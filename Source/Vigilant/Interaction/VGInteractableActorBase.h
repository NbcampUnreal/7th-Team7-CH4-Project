#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VGInteractable.h"
#include "VGInteractableActorBase.generated.h"

UCLASS()
class VIGILANT_API AVGInteractableActorBase : public AActor, public IVGInteractable
{
	GENERATED_BODY()

public:
	AVGInteractableActorBase();
	
	virtual bool CanInteract_Implementation(AVGCharacterBase* Interactor) const override;
	virtual void OnInteract_Implementation(AVGCharacterBase* Interactor) override;
	virtual void PointOnInteract_Implementation(AVGCharacterBase* Interactor, const FHitResult& HitResult) override;
	
protected:
	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const;
	virtual void OnInteractWith(AVGCharacterBase* Interactor);
	virtual void PointOnInteractWith(AVGCharacterBase* Interactor, const FHitResult& HitResult);
};
