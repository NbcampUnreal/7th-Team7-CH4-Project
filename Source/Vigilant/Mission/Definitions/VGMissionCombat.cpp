#include "VGMissionCombat.h"
#include "Character/VGCharacterBase.h"
#include "Mission/VGMissionSandbag.h"
#include "Common/VGGameplayTags.h"
#include "Character/Component/VGStatComponent.h"

AVGMissionCombat::AVGMissionCombat()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionCombat::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		for (AVGMissionSandbag* Sandbag : MissionSandbags)
		{
			if (Sandbag)
			{
				Sandbag->OnSandbagDefeated.AddDynamic(
					this, &AVGMissionCombat::OnSandbagDefeated);
			}
		}
	}
}

void AVGMissionCombat::OnSandbagDefeated(AVGCharacterBase* LastAttacker)
{
	if (LastAttacker)
	{
		GiveRewardToPlayer(LastAttacker);
	}
	
	if (AreAllSandbagsDefeated())
	{
		CompleteMission();
	}
}

void AVGMissionCombat::GiveRewardToPlayer(AVGCharacterBase* Player)
{
	// TODO: 보상 지금 로직
}

bool AVGMissionCombat::AreAllSandbagsDefeated() const
{
	for (AVGMissionSandbag* Sandbag : MissionSandbags)
	{
		if (!Sandbag) continue;

		UVGStatComponent* StatComp =
			Sandbag->FindComponentByClass<UVGStatComponent>();
		if (StatComp && StatComp->GetIsAlive())
		{
			return false;
		}
	}
	return true;
}