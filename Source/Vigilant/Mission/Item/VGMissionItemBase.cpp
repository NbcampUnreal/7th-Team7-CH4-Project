#include "VGMissionItemBase.h"
#include "Mission/Definitions/VGMissionBase.h"
#include "Character/VGCharacterBase.h"
#include "Net/UnrealNetwork.h"

AVGMissionItemBase::AVGMissionItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AVGMissionItemBase::SetStateTag(FGameplayTag NewStateTag)
{
	if (!HasAuthority())
	{
		return;
	}
	
	ItemStateTag = NewStateTag;
	OnRep_ItemStateTag();
	
	OnItemStateChanged.Broadcast(this, NewStateTag);
}

void AVGMissionItemBase::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Carrier);
	DOREPLIFETIME(ThisClass, ItemStateTag);
}

// -----------------------------------------------
// 줍기 / 내려놓기 — 서버 전용
// -----------------------------------------------

void AVGMissionItemBase::OnPickedUp(AVGCharacterBase* NewCarrier)
{
	if (!HasAuthority())
	{
		return;
	}

	Carrier = NewCarrier;
	
	// Carrier가 변경되었으므로 OnRep 수동 호출
	OnRep_Carrier();
}

void AVGMissionItemBase::OnDropped()
{
	if (!HasAuthority())
	{
		return;
	}

	Carrier = nullptr;
	
	OnRep_Carrier();
}

// -----------------------------------------------
// 리플리케이션 콜백
// -----------------------------------------------

void AVGMissionItemBase::OnRep_Carrier()
{
	// TODO: 캐리 상태 변경에 따른 시각적 피드백 처리
	//       (예: 피킹 이펙트 재생, 아웃라인 제거 등)
}

void AVGMissionItemBase::OnRep_ItemStateTag()
{
	// Todo State 변경에 따른 피드백 처리
}
