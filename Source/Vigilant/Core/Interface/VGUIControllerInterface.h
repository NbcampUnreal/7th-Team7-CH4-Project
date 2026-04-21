// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VGUIControllerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UVGUIControllerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class VIGILANT_API IVGUIControllerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void ShowInteractUI(const FString& InteractText, const FVector& TargetLocation, bool bShow) = 0;
};
