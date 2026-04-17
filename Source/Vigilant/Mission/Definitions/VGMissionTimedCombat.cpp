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
		if (!Sandbag)
		{
			continue;
		}
		
		// Sandbag이 StatComponent를 멤버로 보유하고 있으므로 직접 접근
		UVGStatComponent* StatComp = Sandbag->StatComponent;
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
	// 이미 완료된 미션이면 리셋하지 않음 (race condition 방지)
	if (CurrentStateTag == VigilantMissionTags::MissionCompleted)
	{
		return;
	}
	
	SetMissionState(VigilantMissionTags::MissionInactive);
	for (AVGMissionSandbag* Sandbag : MissionSandbags)
	{
		if (!Sandbag)
		{
			continue;
		}
		Sandbag->ResetSandbag();
	}
	
	ClearContributers();
}