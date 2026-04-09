#include "ANSVGMeleeTrace.h"

#include "Character/VGCharacterBase.h"
#include "Character/Component/VGCombatComponent.h"

void UANSVGMeleeTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (AVGCharacterBase* OwnerCharacter = Cast<AVGCharacterBase>(MeshComp->GetOwner()))
	{
		if (OwnerCharacter->IsLocallyControlled())
		{
			if (UVGCombatComponent* CombatComp = OwnerCharacter->GetCombatComponent())
			{
				CombatComp->StartMeleeTrace();
			}
		}
	}
}

void UANSVGMeleeTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, TotalDuration, EventReference);
	
	if (AVGCharacterBase* OwnerCharacter = Cast<AVGCharacterBase>(MeshComp->GetOwner()))
	{
		if (OwnerCharacter->IsLocallyControlled())
		{
			if (UVGCombatComponent* CombatComp = OwnerCharacter->GetCombatComponent())
			{
				CombatComp->TickMeleeTrace();
			}
		}
	}
}

void UANSVGMeleeTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (AVGCharacterBase* OwnerCharacter = Cast<AVGCharacterBase>(MeshComp->GetOwner()))
	{
		if (OwnerCharacter->IsLocallyControlled())
		{
			if (UVGCombatComponent* CombatComp = OwnerCharacter->GetCombatComponent())
			{
				CombatComp->StopMeleeTrace();
			}
		}
	}
}
