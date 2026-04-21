#include "VGMissionGimmickPressure.h"

#include "Character/VGCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Definitions/VGMissionBase.h"
#include "Components/TimelineComponent.h"

AVGMissionGimmickPressure::AVGMissionGimmickPressure()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GimmickTypeTag = VigilantMissionTags::PressureGimmick;
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	
	// 일단 폰(캐릭터)만 감지 - 추후 특정 클래스를 상속한 오브젝트도 감지가능하게 수정
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	PressTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("PressTimeline"));
}

void AVGMissionGimmickPressure::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(
			this, &AVGMissionGimmickPressure::OnTriggerBoxBeginOverlap);
		TriggerBox->OnComponentEndOverlap.AddDynamic(
			this, &AVGMissionGimmickPressure::OnTriggerBoxEndOverlap);
	}
	
	// 초기 위치 기록 (복원에 사용)
	OriginalRelativeLocation = MeshComponent->GetRelativeLocation();
	
	// 커브 자동 생성 (에디터 미설정 시)
	if (!PressCurve)
	{
		PressCurve = NewObject<UCurveFloat>(this);
		PressCurve->FloatCurve.AddKey(0.f, 0.f);
		PressCurve->FloatCurve.AddKey(0.35f, 1.f); // 빠르게 눌림
	}
 
	FOnTimelineFloat Callback;
	Callback.BindUFunction(this, FName("OnPressTimelineUpdate"));
	PressTimeline->AddInterpFloat(PressCurve, Callback);
}

void AVGMissionGimmickPressure::PlayPressAnimation()
{
	if (!PressTimeline) return;
 
	if (GimmickStateTag == VigilantMissionTags::GimmickActive)
	{
		PressTimeline->PlayFromStart(); // 0→1 : 내려가기
	}
	else if (GimmickStateTag == VigilantMissionTags::GimmickInactive)
	{
		PressTimeline->ReverseFromEnd(); // 1→0 : 올라오기
	}
}

void AVGMissionGimmickPressure::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
	
	// Todo : 발판 눌림 /해제 시각 피드백

	PlayPressAnimation();
}

void AVGMissionGimmickPressure::OnPressed()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (GimmickStateTag == VigilantMissionTags::GimmickCompleted)
	{
		return;
	}
	
	SetStateTag(VigilantMissionTags::GimmickActive);
}

void AVGMissionGimmickPressure::OnReleased()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (bToggleMode)
	{
		return;
	}
	
	// 이미 완료된 경우 상태 되돌리지 않음
	if (GimmickStateTag == VigilantMissionTags::GimmickCompleted)
	{
		return;
	}
	
	if (bRevertOnRelease)
	{
		SetStateTag(VigilantMissionTags::GimmickInactive);
	}
}

void AVGMissionGimmickPressure::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(TargetClass) && OtherActor->IsA(TargetClass))
	{
		OverlappingActors.Add(OtherActor);
		OverlappingActors.RemoveAll(
			[](const TObjectPtr<AActor>& A)
			{
				return !IsValid(A);
			}
		);
		
		if (OverlappingActors.Num() >= RequiredActorCount)
		{
			if (bToggleMode &&
				GimmickStateTag == VigilantMissionTags::GimmickActive)
			{
				SetStateTag(VigilantMissionTags::GimmickInactive);
			}
			else
			{
				OnPressed();
			}
		}
		
		// 상호작용 호출이 없으니 직접 호출
		if (AVGCharacterBase* Player = Cast<AVGCharacterBase>(OtherActor))
		{
			OnGimmickInteracted.Broadcast(this, Player);
		}
	}
}


void AVGMissionGimmickPressure::OnTriggerBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsValid(TargetClass) && OtherActor->IsA(TargetClass))
	{
		OverlappingActors.Remove(OtherActor);
	
		OnReleased();
	}
}

void AVGMissionGimmickPressure::OnPressTimelineUpdate(float Value)
{
	// Value 0 = 원래 위치, Value 1 = PressDepth 만큼 아래
	FVector NewLocation = OriginalRelativeLocation;
	NewLocation.Z -= PressDepth * Value;
	MeshComponent->SetRelativeLocation(NewLocation);
}

