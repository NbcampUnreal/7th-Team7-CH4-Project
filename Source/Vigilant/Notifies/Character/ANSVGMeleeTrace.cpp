#include "ANSVGMeleeTrace.h"
#include "Character/Component/VGCombatComponent.h"

void UANSVGMeleeTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UVGCombatComponent* CombatComp = Owner->FindComponentByClass<UVGCombatComponent>())
		{
			CombatComp->StartMeleeTrace();
		}
	}
}

void UANSVGMeleeTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, TotalDuration, EventReference);
	
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UVGCombatComponent* CombatComp = Owner->FindComponentByClass<UVGCombatComponent>())
		{
			CombatComp->TickMeleeTrace();
		}
	}
}

void UANSVGMeleeTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UVGCombatComponent* CombatComp = Owner->FindComponentByClass<UVGCombatComponent>())
		{
			CombatComp->StopMeleeTrace();
		}
	}
}
