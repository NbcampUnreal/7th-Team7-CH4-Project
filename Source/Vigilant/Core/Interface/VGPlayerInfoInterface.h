// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VGPlayerInfoInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UVGPlayerInfoInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class VIGILANT_API IVGPlayerInfoInterface
{
	GENERATED_BODY()
public:
	virtual int32 GetPlayerIndex() const = 0;
	virtual int32 GetRandomMeshNumber() const =0;

};
