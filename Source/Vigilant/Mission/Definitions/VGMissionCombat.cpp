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
		LastContributor = LastAttacker;
	}
	
	if (AreAllSandbagsDefeated())
	{
		CompleteMission();
	}
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