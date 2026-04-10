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
	
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor, const FTransform& InteractTransform) override;
	
protected:
	virtual bool CanInteractWith(AActor* Interactor) const;
	virtual void OnInteractWith(AActor* Interactor, const FTransform& InteractTransform);
};
