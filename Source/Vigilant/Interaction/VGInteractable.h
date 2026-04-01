// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VGInteractable.generated.h"

class AVGCharacterBase;

UINTERFACE(MinimalAPI)
class UVGInteractable : public UInterface
{
	GENERATED_BODY()
};


class VIGILANT_API IVGInteractable
{
	GENERATED_BODY()
	
public:
	virtual bool CheckInteract(AVGCharacterBase* Interactor) const = 0;
	virtual void OnInteract(AVGCharacterBase* Interactor) = 0;
	
};
