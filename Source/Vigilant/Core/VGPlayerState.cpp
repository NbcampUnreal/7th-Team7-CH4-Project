#include "Core/VGPlayerState.h"
#include "Net/UnrealNetwork.h"

AVGPlayerState::AVGPlayerState()
{
	// [Fix] APlayerState는 기본적으로 bReplicates = true — 중복 설정 제거
}

void AVGPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AVGPlayerState, bIsReady);
	// [Fix] PlayerTags 리플리케이션 등록 — 역할/상태 태그를 클라이언트에 동기화
	DOREPLIFETIME(AVGPlayerState, PlayerTags);
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



