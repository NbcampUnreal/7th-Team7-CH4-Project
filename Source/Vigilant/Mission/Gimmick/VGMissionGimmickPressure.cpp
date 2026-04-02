#include "VGMissionGimmickPressure.h"
#include "Components/BoxComponent.h"
#include "Common/VGGameplayTags.h"

AVGMissionGimmickPressure::AVGMissionGimmickPressure()
{
	PrimaryActorTick.bCanEverTick = false;
	
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	
	// 일단 폰(캐릭터)만 감지 - 추후 특정 클래스를 상속한 오브젝트도 감지가능하게 수정
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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
}

bool AVGMissionGimmickPressure::CanInteractWith(AVGCharacterBase* Interactor) const
{
	return false;
}

void AVGMissionGimmickPressure::OnInteractWith(AVGCharacterBase* Interactor)
{
	// 발판은 직접 상호작용이 없습니다.
}

void AVGMissionGimmickPressure::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
	
	// Todo : 발판 눌림 /해제 시각 피드백
	
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
	
	SetGimmickState(VigilantMissionTags::GimmickActive);

	ReportConditionMet();
}

void AVGMissionGimmickPressure::OnReleased()
{
	if (!HasAuthority())
	{
		return;
	}

	// 이미 완료된 경우 상태 되돌리지 않음
	if (GimmickStateTag == VigilantMissionTags::GimmickCompleted)
	{
		return;
	}

	SetGimmickState(VigilantMissionTags::GimmickInative);
}

void AVGMissionGimmickPressure::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(TargetClass) && OtherActor->IsA(TargetClass))
	{
		OverlappingActors.Add(OtherActor);
	
		if (OverlappingActors.Num() >= RequiredActorCount)
		{
			OnPressed();
		}
	}
}


void AVGMissionGimmickPressure::OnTriggerBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsValid(TargetClass) && OtherActor->IsA(TargetClass))
	{
		OverlappingActors.Remove(OtherActor);
	
		if (OverlappingActors.Num() < RequiredActorCount)
		{
			OnReleased();
		}
	}
}

