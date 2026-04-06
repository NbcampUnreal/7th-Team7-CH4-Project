// Fill out your copyright notice in the Description page of Project Settings.


#include "ANSVGIframe.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/CharacterInterface/VGCharacterGameplayTagEditor.h"
#include "Common/VGGameplayTags.h"

void UANSVGIframe::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	AActor* OwnerActor= MeshComp->GetOwner();
	//무적태그 추가
	if (IVGCharacterGameplayTagEditor* CharacterTagEditor = Cast<IVGCharacterGameplayTagEditor>(OwnerActor))
	{
		CharacterTagEditor->AddGameplayTag(VigilantCharacter::Invincible);
	}
	
	
}

void UANSVGIframe::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	AActor* OwnerActor= MeshComp->GetOwner();
	//무적태그 제거
	if (IVGCharacterGameplayTagEditor* CharacterTagEditor = Cast<IVGCharacterGameplayTagEditor>(OwnerActor))
	{
		CharacterTagEditor->RemoveGameplayTag(VigilantCharacter::Invincible);
	}
	
}
