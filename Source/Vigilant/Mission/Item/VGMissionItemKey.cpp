#include "VGMissionItemKey.h"
#include "Common/VGGameplayTags.h"

AVGMissionItemKey::AVGMissionItemKey()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionItemKey::OnRep_ItemStateTag()
{
	Super::OnRep_ItemStateTag();
	if (ItemStateTag == VigilantMissionTags::ItemUsed)
	{
		// 클라이언트는 시각 효과만, 실제 제거는 서버에서
		if (HasAuthority())
		{
			// 한 프레임 뒤에 제거 — 현재 프레임 콜스택 안전하게 완료 후
			SetLifeSpan(0.01f);
		}
	}
}
