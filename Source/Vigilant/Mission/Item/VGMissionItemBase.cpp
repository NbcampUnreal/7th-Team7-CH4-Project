#include "VGMissionItemBase.h"
#include "Mission/VGMissionBase.h"
#include "Character/VGCharacterBase.h"
#include "Net/UnrealNetwork.h"

AVGMissionItemBase::AVGMissionItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

bool AVGMissionItemBase::CanInteractWith(AVGCharacterBase* Interactor) const
{
	return Super::CanInteractWith(Interactor);
}

void AVGMissionItemBase::OnInteractWith(AVGCharacterBase* Interactor)
{
	Super::OnInteractWith(Interactor);
}

void AVGMissionItemBase::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Carrier);
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

	// 아이템이 캐릭터를 따라 움직이도록 Attach
	// TODO: EquipComponent와 연계할 것
	
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

	// 아이템을 월드에 분리
	// TODO: EquipComponent와 연계할 것

	// TODO: 내려놓을 위치 보정 (캐릭터 발 앞 등) 필요 시 SetActorLocation 추가

	OnRep_Carrier();
}

// -----------------------------------------------
// 조건 충족 보고 — 자식 클래스에서 호출
// -----------------------------------------------

void AVGMissionItemBase::ReportConditionMet()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OwnerMission)
	{
		return;
	}

	OwnerMission->OnConditionMet(this);
}

// -----------------------------------------------
// 리플리케이션 콜백
// -----------------------------------------------

void AVGMissionItemBase::OnRep_Carrier()
{
	// TODO: 캐리 상태 변경에 따른 시각적 피드백 처리
	//       (예: 피킹 이펙트 재생, 아웃라인 제거 등)
}