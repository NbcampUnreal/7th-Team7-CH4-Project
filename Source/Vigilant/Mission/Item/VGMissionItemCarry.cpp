#include "VGMissionItemCarry.h"
#include "Common/VGGameplayTags.h"
#include "Mission/Gimmick/VGMissionGimmickBase.h"
#include "Net/UnrealNetwork.h"

AVGMissionItemCarry::AVGMissionItemCarry()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMissionItemCarry::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, AttachmentTargetActor);
	DOREPLIFETIME(ThisClass, AttachmentRelativeLocation);
}

void AVGMissionItemCarry::PlaceOnTarget(AVGMissionGimmickBase* TargetGimmick, FVector TargetRelativeLocation)
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootComp->SetSimulatePhysics(false);
		RootComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// 캐릭터에서 분리 후 Gimmick에 부착
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	

	Carrier = nullptr;
	OnRep_Carrier();
	SetStateTag(VigilantMissionTags::ItemPlaced);
	
	AttachmentTargetActor = TargetGimmick;
	OnRep_AttachmentTargetActor();
	
	AttachmentRelativeLocation = TargetRelativeLocation;
	OnRep_AttachmentRelativeLocation();
	
	UE_LOG(LogTemp, Log, TEXT("[%s] item placed!"),*GetName());
}

void AVGMissionItemCarry::OnRep_AttachmentTargetActor()
{
	AttachToActor(AttachmentTargetActor,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void AVGMissionItemCarry::OnRep_AttachmentRelativeLocation()
{
	if (AttachmentTargetActor)
	{
		this->SetActorRelativeLocation(AttachmentRelativeLocation);
		SetActorRotation(FRotator(0.f));
	}
}

