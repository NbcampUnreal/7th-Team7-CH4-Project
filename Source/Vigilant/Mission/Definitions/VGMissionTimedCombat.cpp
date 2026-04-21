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
		GetWorldTimerManager().ClearTimer(MissionTimerHandle);
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
	if (CurrentStateTag == VigilantMissionTags::MissionCompleted)
	{
		return;
	}
	if (bIsResetting)
	{
		return;
	}
	
	bIsResetting = true;
	SetMissionState(VigilantMissionTags::MissionInactive);
	for (AVGMissionSandbag* Sandbag : MissionSandbags)
	{
		if (!Sandbag)
		{
			continue;
		}
		
		Sandbag->ResetSandbag();
	}
	
	bIsResetting = false;
	UE_LOG(LogTemp, Log, TEXT("[%s] Timed Up!"), *GetName());
	ClearContributers();
}