#include "Combat/VGHitscanExecution.h"

#include "Character/Component/VGCombatComponent.h"
#include "GameFramework/Character.h"

void UVGHitscanExecution::StartAttack()
{
	Super::StartAttack();

	if (!CombatComponent.IsValid())
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(CombatComponent->GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PlayerController)
	{
		return;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * TraceDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	FHitResult HitResult;
	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Pawn, QueryParams);

	// --- Debug ---
	DrawDebugLine(World, TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 2.0f, 0, 1.0f);
	
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			CombatComponent->Server_ProcessHit(HitActor);
			// TODO: 더미 화살 스폰
		}
	}
	
	// TODO: 화살 개수 감소 로직
}
