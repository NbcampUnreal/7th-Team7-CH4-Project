#pragma once

#include "CoreMinimal.h"
#include "Interaction/VGInteractableActorBase.h"
#include "VGEquippableActor.generated.h"

class UVGEquipmentDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemConsumedSignature, AVGEquippableActor*, ConsumedItem);

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
	
	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnItemConsumedSignature OnItemConsumed;
};