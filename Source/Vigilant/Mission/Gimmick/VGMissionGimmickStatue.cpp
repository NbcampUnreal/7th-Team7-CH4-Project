#include "VGMissionGimmickStatue.h"

#include "Common/VGGameplayTags.h"
#include "Net/UnrealNetwork.h"

AVGMissionGimmickStatue::AVGMissionGimmickStatue()
{
	// [Fix] bCanEverTick이 false면 SetActorTickEnabled(true)가 무시되어 회전이 동작하지 않음
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	GimmickTypeTag = VigilantMissionTags::StatueGimmick;
}

bool AVGMissionGimmickStatue::CanInteractWith(AVGCharacterBase* Interactor) const
{
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
	
	if (!CanInteractWith(Interactor))
	{
		return;
	}
	
	// 회전 목표각도 업데이트
	TargetAngle = FMath::Fmod(TargetAngle + RotateStep, 360.f);
	
	// 서버 직접 호출
	OnRep_TargetAngle();
	
	// 회전 시작을 외부에 알림
	SetStateTag(VigilantMissionTags::GimmickActive);
}

void AVGMissionGimmickStatue::BeginPlay()
{
	Super::BeginPlay();
}

void AVGMissionGimmickStatue::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RotateToTarget(DeltaTime);
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

void AVGMissionGimmickStatue::RotateToTarget(float DeltaTime)
{
	if (GimmickStateTag != VigilantMissionTags::GimmickActive)
	{
		return;
	}
	
	FRotator Current = GetActorRotation();
	FRotator Target = FRotator(Current.Pitch, TargetAngle, Current.Roll);
	FRotator New = FMath::RInterpConstantTo(Current, Target, DeltaTime, RotationSpeed);
	SetActorRotation(New);
	
	// 목표 각도 도달 시 회전 완료
	// FRotator::NormalizeAxis : 각도 값을 -180~180 범위로 정규화해줍니다.
	float YawDiff = FMath::Abs(FRotator::NormalizeAxis(New.Yaw - TargetAngle));
	if (YawDiff < AngleTolerance)
	{
		SetActorRotation(Target);
		SetActorTickEnabled(false);
		
		if (HasAuthority())
		{
			if (IsAtAnswerAngle())
			{
				SetStateTag(VigilantMissionTags::GimmickCompleted);
			}
			else
			{
				// 서버에서만 정답 체크
				SetStateTag(VigilantMissionTags::GimmickInactive);
			}
		}
	}
}

bool AVGMissionGimmickStatue::IsAtAnswerAngle() const
{
	// FRotator::NormalizeAxis : 각도 값을 -180~180 범위로 정규화해줍니다.
	float Diff = FMath::Abs(FRotator::NormalizeAxis(TargetAngle - AnswerAngle));
	return Diff   < AngleTolerance;
}

void AVGMissionGimmickStatue::OnRep_TargetAngle()
{
	SetActorTickEnabled(true);
}
