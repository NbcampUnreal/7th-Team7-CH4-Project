#pragma once

#include "CoreMinimal.h"
#include "Equipment/VGEquippableActor.h"
#include "VGWeapon.generated.h"

class UStaticMeshComponent;

UCLASS()
class VIGILANT_API AVGWeapon : public AVGEquippableActor
{
	GENERATED_BODY()
	
public:
	AVGWeapon();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	UStaticMeshComponent* GetWeaponMesh() const {return WeaponMesh;}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
};
