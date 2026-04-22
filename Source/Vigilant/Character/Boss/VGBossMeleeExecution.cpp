// Fill out your copyright notice in the Description page of Project Settings.


#include "VGBossMeleeExecution.h"
#include "Character/Component/VGCombatComponent.h"
#include "Data/VGWeaponDataAsset.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UVGBossMeleeExecution::StartAttack()
{
	Super::StartAttack();
	BossHitActorsThisSwing.Empty();
	BossPrevSockets.Empty();
	bIsFirstTick = true;
}

void UVGBossMeleeExecution::TickAttack()
{
	UVGCombatComponent* CombatComp = Cast<UVGCombatComponent>(GetOuter());
	if (!CombatComp)
	{
		return;
	}
	UMeshComponent* WeaponMesh = CombatComp->GetActiveTraceMesh();
	UVGWeaponDataAsset* Data = CombatComp->GetCurrentCombatData();
	if (!WeaponMesh || !Data)
	{
		return;
	}
	const TArray<FName>& Sockets = Data->HitboxSocketNames;
	if (Sockets.Num() < 2) return;

	if (bIsFirstTick)
    {
        for (const FName& SocketName : Sockets)
        {
            BossPrevSockets.Add(SocketName, WeaponMesh->GetSocketLocation(SocketName));
        }
        bIsFirstTick = false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(CombatComp->GetOwner());

	int32 TimeSegments = 3; 

	FVector BossCenter = CombatComp->GetOwner()->GetActorLocation();
    
	FName HeadSocket = Sockets[Sockets.Num() - 1];
	FVector PrevHead = BossPrevSockets[HeadSocket];
	FVector CurrHead = WeaponMesh->GetSocketLocation(HeadSocket);

	float HitZHeight = BossCenter.Z + 80.0f;
	PrevHead.Z = HitZHeight;
	CurrHead.Z = HitZHeight;

	FVector InnerPoint = BossCenter;
	InnerPoint.Z = HitZHeight;

	for (int32 j = 0; j < TimeSegments; j++)
	{
		float AlphaStart = (float)j / TimeSegments;
		float AlphaEnd = (float)(j + 1) / TimeSegments;

		FVector StartHead = FMath::Lerp(PrevHead, CurrHead, AlphaStart);
		FVector EndHead = FMath::Lerp(PrevHead, CurrHead, AlphaEnd);

		FVector StartMid = (InnerPoint + StartHead) / 2.0f; 
		FVector EndMid = (InnerPoint + EndHead) / 2.0f;

		float GiantRadius = (FVector::Distance(InnerPoint, StartHead) / 2.0f) + AttackRadius;
		TArray<FHitResult> HitResults;
		
		bool bSegmentHit = GetWorld()->SweepMultiByChannel(
			HitResults,
			StartMid,
			EndMid, 
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(GiantRadius), 
			QueryParams
		);

		if (bSegmentHit)
		{
			for (const FHitResult& Hit : HitResults)
			{
				AActor* HitActor = Hit.GetActor();
				APawn* HitPawn = Cast<APawn>(HitActor);
				
				if (HitPawn && !BossHitActorsThisSwing.Contains(HitPawn))
				{
					BossHitActorsThisSwing.Add(HitPawn);
					CombatComp->Server_ProcessHit(HitPawn);
					
					if (HitSound)
					{
						UGameplayStatics::PlaySoundAtLocation(
							GetWorld(), 
							HitSound, 
							Hit.ImpactPoint
						);
					}
				}
			}
		}
	}

    for (const FName& SocketName : Sockets)
    {
        BossPrevSockets[SocketName] = WeaponMesh->GetSocketLocation(SocketName);
    }
}
