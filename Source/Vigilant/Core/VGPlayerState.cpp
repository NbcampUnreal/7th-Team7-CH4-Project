#include "Core/VGPlayerState.h"

#include "Interface/VGUIControllerInterface.h"
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
	DOREPLIFETIME(AVGPlayerState, AssignedMeshIndex);
}

void AVGPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	// 게임이 초기화될 때 새로운 PlayerState에 복사
	if (AVGPlayerState* NewVGPlayerState = Cast<AVGPlayerState>(PlayerState))
	{
		// 닉네임 복사
		NewVGPlayerState->VGPlayerName = this->VGPlayerName;
        
		UE_LOG(LogTemp, Warning, TEXT("[VGPlayerState] 닉네임 복사 완료: %s"), *this->VGPlayerName);
	}
}

void AVGPlayerState::Client_ReceiveRole_Implementation(FGameplayTag AssignedRoleTag)
{
	if (AssignedRoleTag.IsValid())
	{
		SecretRoleTag = AssignedRoleTag;
	}
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetOwner()))
	{
		// 인터페이스를 통해 UI에 직업 알림 지시
		if (IVGUIControllerInterface* UIController = Cast<IVGUIControllerInterface>(PlayerController))
		{
			UIController->ShowRoleNotificationUI(AssignedRoleTag);
		}
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

bool AVGPlayerState::IsRole(const FGameplayTag& RoleTagToCheck) const
{
	return SecretRoleTag == RoleTagToCheck;
}

void AVGPlayerState::SetVGPlayerName(const FString& NewName)
{
	VGPlayerName = NewName;
}

void AVGPlayerState::OnRep_VGPlayerName()
{
	if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
	{
		if (IVGUIControllerInterface* UIController = Cast<IVGUIControllerInterface>(LocalPC))
		{
			// 몇 번째 플레이어(EntryIndex)인지와 이름 데이터를 인터페이스로 토스
			UIController->UpdatePlayerNameUI(EntryIndex, VGPlayerName);
		}
	}
}

int32 AVGPlayerState::GetPlayerIndex() const
{
	return EntryIndex;
}

int32 AVGPlayerState::GetRandomMeshNumber() const
{
	return AssignedMeshIndex;
}



