// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "Character/VGCharacterBase.h"
#include "Character/CharacterInterface/VGCharacterGameplayTagEditor.h"
#include "VGCitizenCharacter.generated.h"

/**
 * 
 */
UCLASS()
class VIGILANT_API AVGCitizenCharacter : 
public AVGCharacterBase, 
public IVGCharacterGameplayTagEditor,
public IGameplayTagAssetInterface

{
	GENERATED_BODY()
	
	
#pragma region Interfaces Func
public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual void AddGameplayTag(FGameplayTag TagToAdd) override;
	virtual void RemoveGameplayTag(FGameplayTag TagToRemove) override;
#pragma endregion 김형백
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Tags")
	FGameplayTagContainer CharacterGameplayTags;
};
