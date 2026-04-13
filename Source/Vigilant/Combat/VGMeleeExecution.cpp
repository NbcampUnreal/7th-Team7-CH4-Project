#include "Combat/VGMeleeExecution.h"
#include "Character/Component/VGCombatComponent.h"
#include "Data/VGWeaponDataAsset.h"
#include "GameFramework/Character.h"

void UVGMeleeExecution::StartAttack()
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

	HitActorsThisSwing.Empty();
	PreviousSocketLocations.Empty();

	UVGWeaponDataAsset* Data = CombatComponent->GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	UMeshComponent* TraceMesh = CombatComponent->GetActiveTraceMesh();
	
	if (!TraceMesh)
	{
		TraceMesh = OwnerCharacter->GetMesh();

	}
	
	if (!TraceMesh)
	{
		return;
	}

	// Data Asset에 정의된 모든 소켓의 시작 위치 기록
	for (const FName& SocketName : Data->HitboxSocketNames)
	{
		FVector StartLoc = TraceMesh->GetSocketLocation(SocketName);
		PreviousSocketLocations.Add(SocketName, StartLoc);
	}
}

void UVGMeleeExecution::TickAttack()
{
	Super::TickAttack();
	
	if (!CombatComponent.IsValid())
	{
		return;
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(CombatComponent->GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	UVGWeaponDataAsset* Data = CombatComponent->GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	UMeshComponent* TraceMesh = CombatComponent->GetActiveTraceMesh();
	if (!TraceMesh)
	{
		TraceMesh = OwnerCharacter->GetMesh();
	}

	if (!TraceMesh)
	{
		return;
	}

	FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	// QueryParams.bTraceComplex = true;

	for (const FName& SocketName : Data->HitboxSocketNames)
	{
		if (!PreviousSocketLocations.Contains(SocketName))
		{
			continue;
		}

		FVector PreviousLoc = PreviousSocketLocations[SocketName];
		FVector CurrentLoc = TraceMesh->GetSocketLocation(SocketName);

		TArray<FHitResult> HitResults;

		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}
		
		bool bHit = World->SweepMultiByChannel(
			HitResults,
			PreviousLoc,
			CurrentLoc,
			FQuat::Identity,
			ECC_Pawn,
			Sphere,
			QueryParams
		);

		// --- Debug ---
		FVector TraceVector = CurrentLoc - PreviousLoc;
		float TraceLength = TraceVector.Size();
		if (TraceLength > KINDA_SMALL_NUMBER)
		{
			FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();
			DrawDebugCapsule(
				GetWorld(),
				PreviousLoc + (TraceVector * 0.5f),
				(TraceLength * 0.5f) + AttackRadius,
				AttackRadius,
				CapsuleRot,
				bHit ? FColor::Green : FColor::Red,
				false,
				2.0f
			);
		}
		// ---

		if (bHit)
		{
			for (const FHitResult& HitResult : HitResults)
			{
				AActor* HitActor = HitResult.GetActor();
				if (HitActor && !HitActorsThisSwing.Contains(HitActor))
				{
					HitActorsThisSwing.Add(HitActor);
					CombatComponent->Server_ProcessHit(HitActor);
				}
			}
		}

		PreviousSocketLocations[SocketName] = CurrentLoc;
	}
}

void UVGMeleeExecution::StopAttack()
{
	Super::StopAttack();
	
	if (!CombatComponent.IsValid())
	{
		return;
	}
	
	APawn* OwnerPawn = Cast<APawn>(CombatComponent->GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	HitActorsThisSwing.Empty();
	PreviousSocketLocations.Empty();
}
