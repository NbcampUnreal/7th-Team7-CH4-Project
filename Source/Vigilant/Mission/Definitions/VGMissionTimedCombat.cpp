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

void AVGMissionTimedCombat::SpawnRewardItems()
{
	// 스폰은 서버에서만 진행
	if (!HasAuthority())
	{
		return;
	}
	
	// 기본 구현: LastContributor 주변에 아이템 스폰
	// 자식 클래스에서 override하여 커스텀
	if (GetRewardItemClass() == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RewardItemClass is Missing."));
		return;
	}
	
	FVector SpawnLocation = GetActorLocation()
						  + GetActorForwardVector() * 100.f;
	SpawnLocation.Z += 50.f;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<AVGEquippableActor>(GetRewardItemClass(), SpawnLocation,
								   FRotator::ZeroRotator, Params);
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
	GetWorldTimerManager().ClearTimer(SandbagTimerHandle);
		
	for (AVGMissionSandbag* Sandbag : MissionSandbags)
	{
		if (!Sandbag) continue;
		Sandbag->ResetSandbag();
	}
}