#include "VGMissionGimmickLever.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"

AVGMissionGimmickLever::AVGMissionGimmickLever()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AVGMissionGimmickLever::IsActivated() const
{
	return (GimmickStateTag == VigilantMissionTags::GimmickActive);
}

bool AVGMissionGimmickLever::CanInteractWith(AVGCharacterBase* Interactor) const
{
	if (bIsOneWay && IsActivated())
	{
		return false;
	}
	
	return true;
}

void AVGMissionGimmickLever::OnInteractWith(AVGCharacterBase* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	Toggle();
	// 상호작용한 플레이어에게만 이펙트 요청
	// Interactor의 PlayerController를 통해 Client RPC 호출
	if (APlayerController* PC =
		Cast<APlayerController>(Interactor->GetController()))
	{
		// PC->Client_PlayInteractEffect(GetActorLocation());
	}
}

void AVGMissionGimmickLever::Toggle()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (IsActivated())
	{
		SetStateTag(VigilantMissionTags::GimmickInactive);
	}
	else
	{
		SetStateTag(VigilantMissionTags::GimmickActive);
	}
}

void AVGMissionGimmickLever::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
}
