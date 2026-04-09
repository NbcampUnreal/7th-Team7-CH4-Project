// Fill out your copyright notice in the Description page of Project Settings.


#include "ANSVGGamplayTag.h"

#include "Character/VGCharacterBase.h"

void UANSVGGamplayTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (AVGCharacterBase* Character = Cast<AVGCharacterBase>(MeshComp->GetOwner()))
	{
		if (Character->IsLocallyControlled() || Character->HasAuthority())
		{
			Character->AddGameplayTag(TagToApply);
		}
	}
}

void UANSVGGamplayTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (AVGCharacterBase* Character = Cast<AVGCharacterBase>(MeshComp->GetOwner()))
	{
		if (Character->IsLocallyControlled() || Character->HasAuthority())
		{
			Character->RemoveGameplayTag(TagToApply);
		}
	}
}
