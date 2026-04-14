#pragma once

#include "CoreMinimal.h"
#include "Interaction/VGInteractableActorBase.h"
#include "VGEquippableActor.generated.h"

class UVGEquipmentDataAsset;

UCLASS()
class VIGILANT_API AVGEquippableActor : public AVGInteractableActorBase
{
	GENERATED_BODY()
	
public:
	AVGEquippableActor();
	
	virtual UMeshComponent* GetItemMesh() const { return nullptr; }

protected:
	virtual void OnInteractWith(AActor* Interactor, const FTransform& InteractTransform) override;

public:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UVGEquipmentDataAsset> EquipmentData;
};
