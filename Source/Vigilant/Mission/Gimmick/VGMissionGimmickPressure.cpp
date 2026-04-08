#include "VGMissionGimmickPressure.h"

#include "Character/VGCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Definitions/VGMissionBase.h"

AVGMissionGimmickPressure::AVGMissionGimmickPressure()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GimmickTypeTag = VigilantMissionTags::PressureGimmick;
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

void AVGMissionGimmickPressure::OnRep_GimmickStateTag()
{
	Super::OnRep_GimmickStateTag();
	
	// Todo : 발판 눌림 /해제 시각 피드백
	UE_LOG(LogTemp, Warning, TEXT("[Pressure:%s] CurrentState : %s"), *GetName(), *GimmickStateTag.ToString());
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

	UE_LOG(LogTemp, Warning, TEXT("[Pressure:%s] OnPressed"), *GetName());
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
	
	UE_LOG(LogTemp, Warning, TEXT("[Pressure:%s] OnReleased"), *GetName());
	SetStateTag(VigilantMissionTags::GimmickInactive);
}

void AVGMissionGimmickPressure::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(TargetClass) && OtherActor->IsA(TargetClass))
	{
		OverlappingActors.Add(OtherActor);
		
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
	
		if (OverlappingActors.Num() < RequiredActorCount)
		{
			OnReleased();
		}
	}
}

