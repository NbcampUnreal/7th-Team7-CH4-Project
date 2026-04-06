#include "Core/VGPlayerState.h"
#include "Net/UnrealNetwork.h"

AVGPlayerState::AVGPlayerState()
{
	bReplicates = true;
}

void AVGPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AVGPlayerState, bIsReady);
}

void AVGPlayerState::Client_ReceiveRole_Implementation(FGameplayTag AssignedRoleTag)
{
	if (AssignedRoleTag.IsValid())
	{
		PlayerTags.AddTag(AssignedRoleTag);
	}
}

void AVGPlayerState::AddPlayerTag(const FGameplayTag& TagToAdd)
{
	if (TagToAdd.IsValid())
	{
		PlayerTags.AddTag(TagToAdd);
	}
}

void AVGPlayerState::RemovePlayerTag(const FGameplayTag& TagToRemove)
{
	if (TagToRemove.IsValid())
	{
		PlayerTags.RemoveTag(TagToRemove);
	}
}

bool AVGPlayerState::HasPlayerTag(const FGameplayTag& TagToCheck) const
{
	if (TagToCheck.IsValid())
	{
		return PlayerTags.HasTag(TagToCheck);
	}
	
	return false;
}



