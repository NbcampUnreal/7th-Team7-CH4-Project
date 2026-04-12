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
	AttachmentRelativeLocation = TargetRelativeLocation;
	
	OnRep_AttachmentTargetActor();
	OnRep_AttachmentRelativeLocation();
	
	UE_LOG(LogTemp, Log, TEXT("[%s] item placed!"),*GetName());
}

void AVGMissionItemCarry::OnRep_AttachmentTargetActor()
{
	if (!AttachmentTargetActor)
	{
		return;
	}
	
	AttachToActor(AttachmentTargetActor,
		FAttachmentTransformRules::KeepWorldTransform);
	
	SetActorRelativeLocation(AttachmentRelativeLocation);
	SetActorRelativeRotation(FRotator::ZeroRotator);
}

void AVGMissionItemCarry::OnRep_AttachmentRelativeLocation()
{
	if (!AttachmentTargetActor || !GetAttachParentActor())
	{
		return;
	}
    
	SetActorRelativeLocation(AttachmentRelativeLocation);
	SetActorRelativeRotation(FRotator::ZeroRotator);
}

void AVGMissionItemCarry::OnRep_Carrier()
{
	if (ItemStateTag == VigilantMissionTags::ItemPlaced)
	{
		// Altar 부착 상태 복구
		if (AttachmentTargetActor)
		{
			AttachToActor(AttachmentTargetActor,
				FAttachmentTransformRules::KeepWorldTransform);
			SetActorRelativeLocation(AttachmentRelativeLocation);
			SetActorRelativeRotation(FRotator::ZeroRotator);
		}

		if (UPrimitiveComponent* RootComp =
			Cast<UPrimitiveComponent>(GetRootComponent()))
		{
			RootComp->SetSimulatePhysics(false);
			RootComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		return;
	}
	
	Super::OnRep_Carrier();
}

