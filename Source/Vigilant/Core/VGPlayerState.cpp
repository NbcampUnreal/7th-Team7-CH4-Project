#include "Core/VGPlayerState.h"
#include "Net/UnrealNetwork.h"

AVGPlayerState::AVGPlayerState()
{
}

void AVGPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AVGPlayerState, bIsReady);
	DOREPLIFETIME(AVGPlayerState, PlayerStatusTags);
	DOREPLIFETIME(AVGPlayerState, VGPlayerName);
	DOREPLIFETIME(AVGPlayerState, EntryIndex);
}

void AVGPlayerState::Client_ReceiveRole_Implementation(FGameplayTag AssignedRoleTag)
{
	if (AssignedRoleTag.IsValid())
	{
		PlayerStatusTags.AddTag(AssignedRoleTag);
	}
}

void AVGPlayerState::AddPlayerTag(const FGameplayTag& TagToAdd)
{
	if (TagToAdd.IsValid())
	{
		PlayerStatusTags.AddTag(TagToAdd);
	}
}

void AVGPlayerState::RemovePlayerTag(const FGameplayTag& TagToRemove)
{
	if (TagToRemove.IsValid())
	{
		PlayerStatusTags.RemoveTag(TagToRemove);
	}
}

bool AVGPlayerState::HasPlayerTag(const FGameplayTag& TagToCheck) const
{
	if (TagToCheck.IsValid())
	{
		return PlayerStatusTags.HasTag(TagToCheck);
	}
	
	return false;
}



