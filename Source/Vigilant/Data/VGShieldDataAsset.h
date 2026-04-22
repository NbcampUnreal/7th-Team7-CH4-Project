#pragma once

#include "CoreMinimal.h"
#include "Data/VGEquipmentDataAsset.h"
#include "VGShieldDataAsset.generated.h"

UCLASS(BlueprintType)
class VIGILANT_API UVGShieldDataAsset : public UVGEquipmentDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield|Animation")
	TObjectPtr<UAnimMontage> BlockMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield|Stats", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageMitigation = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield|Stats")
	float BlockActivationStaminaCost = 10.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield|Stats")
	float BlockStaminaDrainPerSecond = 5.0f;
};
