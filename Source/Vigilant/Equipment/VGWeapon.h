#pragma once

#include "CoreMinimal.h"
#include "Equipment/VGEquippableActor.h"
#include "VGWeapon.generated.h"

class UStaticMeshComponent;
class UVGWeaponDataAsset;

UCLASS()
class VIGILANT_API AVGWeapon : public AVGEquippableActor
{
	GENERATED_BODY()
	
public:
	AVGWeapon();
	
	// CombatComponent가 소켓 위치를 얻을 수 있도록 메쉬를 반환
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	UStaticMeshComponent* GetWeaponMesh() const {return WeaponMesh;}
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	UVGWeaponDataAsset* GetWeaponData() const {return WeaponData;}
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UVGWeaponDataAsset> WeaponData;
};
