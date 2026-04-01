// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "VGEquipmentDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class VIGILANT_API UVGEquipmentDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FGameplayTag SlotTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FGameplayTag ItemTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FName SocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	int32 DropPriority;

	// 월드에 스폰하거나 장착할 실제 블루프린트 액터 클래스 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<AVGEquippableActor> EquippableActorClass;
	
	
};
