#include "VGMissionTimedCombat.h"
#include "Character/VGCharacterBase.h"
#include "Mission/VGMissionSandbag.h"
#include "Common/VGGameplayTags.h"
#include "Character/Component/VGStatComponent.h"

AVGMissionTimedCombat::AVGMissionTimedCombat()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionTimedCombat::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		for (AVGMissionSandbag* Sandbag : MissionSandbags)
		{
			if (Sandbag)
			{
				Sandbag->OnSandbagDefeated.AddDynamic(
					this, &AVGMissionTimedCombat::OnSandbagDefeated);
				
				Sandbag->OnSandbagHitted.AddDynamic(this, 
					&AVGMissionTimedCombat::OnSandbagHitted);
			}
		}
	}
}

void AVGMissionTimedCombat::OnSandbagDefeated(AVGCharacterBase* LastAttacker)
{
	if (LastAttacker)
	{
		RegisterContributor(LastAttacker);
	}
	
	if (AreAllSandbagsDefeated())
	{
		GetWorldTimerManager().ClearTimer(SandbagTimerHandle);
		CompleteMission();
	}
}

void AVGMissionTimedCombat::OnSandbagHitted()
{
	if (CurrentStateTag == VigilantMissionTags::MissionInactive)
	{
		StartTimer();
		SetMissionState(VigilantMissionTags::MissionActive);
	}
}

bool AVGMissionTimedCombat::AreAllSandbagsDefeated() const
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

void AVGMissionTimedCombat::StartTimer()
{
	GetWorldTimerManager().SetTimer(
		MissionTimerHandle,
		this,
		&AVGMissionTimedCombat::OnTimerExpired,
		GetMissionTimeLimit(),  // MissionBase에 이미 있는 변수
		false
	);
}

void AVGMissionTimedCombat::OnTimerExpired()
{
	SetMissionState(VigilantMissionTags::MissionInactive);
	for (AVGMissionSandbag* Sandbag : MissionSandbags)
	{
		if (!Sandbag) continue;
		Sandbag->ResetSandbag();
	}
	
	UE_LOG(LogTemp, Error, TEXT("[%s] Timed Up!"), *GetName());
	ClearContributers();
}