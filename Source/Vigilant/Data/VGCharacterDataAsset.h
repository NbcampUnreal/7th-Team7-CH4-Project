// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VGCharacterDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class VIGILANT_API UVGCharacterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Mesh")
	TArray<TObjectPtr<USkeletalMesh>> PlayerMeshes;
	
	
};
