#include "VGMissionGimmickLever.h"
#include "Common/VGGameplayTags.h"
#include "Character/VGCharacterBase.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Components/TimelineComponent.h"

AVGMissionGimmickLever::AVGMissionGimmickLever()
{
	PrimaryActorTick.bCanEverTick = false;
	GimmickTypeTag = VigilantMissionTags::LeverGimmick;
	
	// 뚜껑 메시 — 피봇을 경첩 위치에 맞춰야 자연스럽게 열림
	LeverMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMeshComponent"));
	LeverMeshComponent->SetupAttachment(MeshComponent);
 
	LeverTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("LidTimeline"));
	
	TargetPitch = InactivePitch;
}

bool AVGMissionGimmickLever::IsActivated() const
{
	return (GimmickStateTag == VigilantMissionTags::GimmickActive);
}

bool AVGMissionGimmickLever::CanInteractWith(AActor* Interactor) const
{
	if (bIsOneWay && IsActivated())
	{
		return false;
	}
	
	return true;
}

void AVGMissionGimmickLever::OnInteractWith(AActor* Interactor, const FTransform& InteractTransform)
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (!CanInteractWith(Interactor))
	{
		return;
	}
	
	Toggle();
	
	OnGimmickInteracted.Broadcast(this, Interactor);
}

void AVGMissionGimmickLever::BeginPlay()
{
	Super::BeginPlay();
	
	// 커브가 없으면 기본 Linear 커브 생성
	if (!LeverRotationCurve)
	{
		LeverRotationCurve = NewObject<UCurveFloat>(this);
		LeverRotationCurve->FloatCurve.AddKey(0.f, 0.f);
		LeverRotationCurve->FloatCurve.AddKey(0.3f, 1.f);
	}
 
	// 타임라인 바인딩
	FOnTimelineFloat ProgressCallback;
	ProgressCallback.BindUFunction(this, FName("OnLeverTimelineUpdate"));
	LeverTimeline->AddInterpFloat(LeverRotationCurve, ProgressCallback);
 
	FOnTimelineEvent FinishedCallback;
	FinishedCallback.BindUFunction(this, FName("OnLeverTimelineFinished"));
	LeverTimeline->SetTimelineFinishedFunc(FinishedCallback);
 
	// 초기 각도 세팅
	LeverMeshComponent->SetRelativeRotation(FRotator(InactivePitch, 0.f, 0.f));
}

void AVGMissionGimmickLever::Toggle()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (IsActivated())
	{
		SetStateTag(VigilantMissionTags::GimmickInactive);
	}
	else
	{
		SetStateTag(VigilantMissionTags::GimmickActive);
	}
}

void AVGMissionGimmickLever::OnLeverTimelineUpdate(float Value)
{
	// Value: 0(InactivePitch) ~ 1(ActivePitch)
	float CurrentPitch = FMath::Lerp(InactivePitch, ActivePitch, Value);
	LeverMeshComponent->SetRelativeRotation(FRotator(CurrentPitch, 0.f, 0.f));
}

void AVGMissionGimmickLever::OnLeverTimelineFinished()
{
	// 필요 시 추가 작성
}

void AVGMissionGimmickLever::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
	
	PlayLeverAnimation();
}

void AVGMissionGimmickLever::PlayLeverAnimation()
{
	if (!LeverTimeline) return;
 
	if (GimmickStateTag == VigilantMissionTags::GimmickActive)
	{
		LeverTimeline->PlayFromStart();
	}
	else if (GimmickStateTag == VigilantMissionTags::GimmickInactive)
	{
		LeverTimeline->ReverseFromEnd();
	}
}
