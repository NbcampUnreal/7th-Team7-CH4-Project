// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Citizen/VGCitizenCharacter.h"

void AVGCitizenCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = CharacterGameplayTags
}

void AVGCitizenCharacter::AddGameplayTag(FGameplayTag TagToAdd)
{
	CharacterGameplayTags.AddTag(TagToAdd);
}

void AVGCitizenCharacter::RemoveGameplayTag(FGameplayTag TagToRemove)
{
	CharacterGameplayTags.RemoveTag(TagToRemove);
}
