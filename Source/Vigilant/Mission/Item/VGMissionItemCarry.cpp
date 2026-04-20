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
	
	DOREPLIFETIME(ThisClass, PlaceInfo);
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
	
	FVGCarryPlaceInfo Info;
	Info.AttachmentTargetActor = TargetGimmick;
	Info.RelativeLocation = TargetRelativeLocation;
	PlaceInfo = Info;
	OnRep_PlaceInfo();
	
	SetStateTag(VigilantMissionTags::ItemPlaced);
	UE_LOG(LogTemp, Log, TEXT("[%s] item placed!"),*GetName());
}


void AVGMissionItemCarry::OnRep_PlaceInfo()
{
	// 타깃이 유효하지 않으면 attach가 크래시를 유발할 수 있으므로 방어
	if (!PlaceInfo.AttachmentTargetActor)
	{
		return;
	}
	
	if (UPrimitiveComponent* RootComp =
		Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootComp->SetSimulatePhysics(false);
		RootComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	AttachToActor(PlaceInfo.AttachmentTargetActor,
		FAttachmentTransformRules::KeepWorldTransform);
    	
	SetActorRelativeLocation(PlaceInfo.RelativeLocation);
	SetActorRelativeRotation(FRotator::ZeroRotator);
	
	BP_OnRep_PlaceInfo();
}