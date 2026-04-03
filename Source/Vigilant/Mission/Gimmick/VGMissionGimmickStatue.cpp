#include "VGMissionGimmickStatue.h"

#include "Common/VGGameplayTags.h"
#include "Net/UnrealNetwork.h"

AVGMissionGimmickStatue::AVGMissionGimmickStatue()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

bool AVGMissionGimmickStatue::CanInteractWith(AVGCharacterBase* Interactor) const
{
	if (bIsRotating)
	{
		return false;
	}
	
	if (GimmickStateTag != VigilantMissionTags::GimmickInactive)
	{
		return false;
	}
	
	return true;
}

void AVGMissionGimmickStatue::OnInteractWith(AVGCharacterBase* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}
	
	SetStateTag(VigilantMissionTags::GimmickActive);
}

void AVGMissionGimmickStatue::BeginPlay()
{
	Super::BeginPlay();
}

void AVGMissionGimmickStatue::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bIsRotating) return;

	FRotator Current = GetActorRotation();
	FRotator Target = FRotator(Current.Pitch, TargetAngle, Current.Roll);
	FRotator New = FMath::RInterpConstantTo(Current, Target, DeltaTime, RotationSpeed);
	SetActorRotation(New);
	
	// 목표 각도 도달 시 회전 완료
	if (FMath::Abs(New.Yaw - TargetAngle) < AngleTolerance)
	{
		SetActorRotation(Target);
		bIsRotating = false;
		SetActorTickEnabled(false);

		// 서버에서만 정답 체크
		SetStateTag(VigilantMissionTags::GimmickInactive);
	}
}

void AVGMissionGimmickStatue::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, TargetAngle);
}

void AVGMissionGimmickStatue::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
	if (GimmickStateTag == VigilantMissionTags::GimmickActive)
	{
		
	}
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
	bIsRotating = true;
	SetActorTickEnabled(true);
}
