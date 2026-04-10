#include "VGMissionAllGimmicksClear.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"
#include "Common/VGGameplayTags.h"

AVGMissionAllGimmicksClear::AVGMissionAllGimmicksClear()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionAllGimmicksClear::OnGimmickStateChanged(AVGMissionGimmickBase* Gimmick, FGameplayTag Tag)
{
	Super::OnGimmickStateChanged(Gimmick, Tag);
	if (Tag == VigilantMissionTags::GimmickCompleted)
	{
		if (AreAllGimmickCompleted())
		{
			CompleteMission();
		}
	}
}

void AVGMissionAllGimmicksClear::SpawnRewardItems()
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

bool AVGMissionAllGimmicksClear::AreAllGimmickCompleted() const
{
	for (AVGMissionGimmickBase* Gimmick : MissionGimmicks)
	{
		if (!Gimmick)
		{
			continue;
		}
		
		if (Gimmick->GetStateTag() != VigilantMissionTags::GimmickCompleted)
		{
			return false;
		}
	}
	return true;
}



