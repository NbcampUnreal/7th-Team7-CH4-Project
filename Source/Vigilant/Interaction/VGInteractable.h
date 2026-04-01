// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VGInteractable.generated.h"

class AVGCharacterBase;

// This class does not need to be modified.
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
	bool CanInteract(AVGCharacterBase* Interactor) const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VG|Interact")
	void OnInteract(AVGCharacterBase* Interactor);
};
