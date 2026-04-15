#include "VGMissionRotatingStatue.h"
#include "Mission/Gimmick/VGMissionGimmickStatue.h"
#include "Common/VGGameplayTags.h"

AVGMissionRotatingStatue::AVGMissionRotatingStatue()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionRotatingStatue::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
	Super::OnGimmickStateChanged(Gimmick, Tag);
	
	if (AreAllStatueAligned())
	{
		CompleteMission();
	}
}

void AVGMissionRotatingStatue::SpawnRewardItems()
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

bool AVGMissionRotatingStatue::AreAllStatueAligned() const
{
	for (AVGMissionGimmickBase* Gimmick : MissionGimmicks)
	{
		if (!Gimmick)
		{
			continue;
		}
		
		if (AVGMissionGimmickStatue* Statue = Cast<AVGMissionGimmickStatue>(Gimmick))
		{
			if (!Statue->IsAtAnswerAngle())
			{
				return false;
			}
		}
	}
	return true;
}

