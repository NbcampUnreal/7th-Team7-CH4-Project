#pragma once

#include "CoreMinimal.h"
#include "Equipment/VGEquippableActor.h"
#include "VGShield.generated.h"

UCLASS()
class VIGILANT_API AVGShield : public AVGEquippableActor
{
	GENERATED_BODY()

public:
	AVGShield();

	virtual UMeshComponent* GetItemMesh() const override { return ShieldMesh; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield")
	TObjectPtr<UStaticMeshComponent> ShieldMesh;
};
