#include "Character/Component/VGAnimNotifyState_MeleeTrace.h"
#include "Character/Component/VGCombatComponent.h"

void UVGAnimNotifyState_MeleeTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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

void UVGAnimNotifyState_MeleeTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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

void UVGAnimNotifyState_MeleeTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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
