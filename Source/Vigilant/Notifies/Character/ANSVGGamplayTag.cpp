#include "ANSVGGamplayTag.h"
#include "Character/CharacterInterface/VGCharacterGameplayTagEditor.h"

void UANSVGGamplayTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (IVGCharacterGameplayTagEditor* TagEditor = Cast<IVGCharacterGameplayTagEditor>(OwnerActor))
	{
		if (OwnerActor->HasAuthority() || OwnerActor->GetLocalRole() == ROLE_AutonomousProxy)
		{
			TagEditor->AddGameplayTag(TagToApply);
		}
	}
}

void UANSVGGamplayTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (IVGCharacterGameplayTagEditor* TagEditor = Cast<IVGCharacterGameplayTagEditor>(OwnerActor))
	{
		if (OwnerActor->HasAuthority() || OwnerActor->GetLocalRole() == ROLE_AutonomousProxy)
		{
			TagEditor->RemoveGameplayTag(TagToApply);
		}
	}
}
