#include "VGMissionItemCarry.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"

AVGMissionItemCarry::AVGMissionItemCarry()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionItemCarry::PlaceOnTarget(AVGMissionGimmickBase* TargetGimmick)
{
	if (!HasAuthority()) return;

	// 캐릭터에서 분리 후 Gimmick에 부착
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	AttachToActor(TargetGimmick,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	Carrier = nullptr;
	SetStateTag(VigilantMissionTags::ItemPlaced);
	OnRep_Carrier();
}

