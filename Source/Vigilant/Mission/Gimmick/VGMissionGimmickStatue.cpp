#include "VGMissionGimmickStatue.h"
#include "Net/UnrealNetwork.h"

AVGMissionGimmickStatue::AVGMissionGimmickStatue()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

bool AVGMissionGimmickStatue::CanInteract_Implementation(AVGCharacterBase* Interactor) const
{
	return IVGInteractable::CanInteract_Implementation(Interactor);
}

void AVGMissionGimmickStatue::OnInteract_Implementation(AVGCharacterBase* Interactor)
{
	IVGInteractable::OnInteract_Implementation(Interactor);
}

void AVGMissionGimmickStatue::BeginPlay()
{
	Super::BeginPlay();
}

void AVGMissionGimmickStatue::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVGMissionGimmickStatue::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, TargetAngle);
}

void AVGMissionGimmickStatue::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
}

void AVGMissionGimmickStatue::RotateToTarget()
{
}

bool AVGMissionGimmickStatue::IsAtAnswerAngle() const
{
	return false;
}

void AVGMissionGimmickStatue::OnRep_TargetAngle()
{
}
