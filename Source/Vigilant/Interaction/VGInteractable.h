#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VGInteractable.generated.h"

UINTERFACE(MinimalAPI)
class UVGInteractable : public UInterface
{
	GENERATED_BODY()
};


class VIGILANT_API IVGInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VG|Interact")
	bool CanInteract(AActor* Interactor) const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VG|Interact")
	void OnInteract(AActor* Interactor, const FTransform& InteractTransform);
};
