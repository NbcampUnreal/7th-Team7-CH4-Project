#include "Combat/VGHitscanExecution.h"

#include "VGAmmoProviderInterface.h"
#include "Character/Component/VGCombatComponent.h"
#include "Data/VGWeaponDataAsset.h"
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
	
	// 1. 로컬 탄약 확인
	UMeshComponent* TraceMesh = CombatComponent->GetActiveTraceMesh();
	if (!TraceMesh)
	{
		return;
	}
	
	if (IVGAmmoProviderInterface* AmmoProvider = Cast<IVGAmmoProviderInterface>(TraceMesh->GetOwner()))
	{
		if (!AmmoProvider->HasAmmo())
		{
			return;
		}
		AmmoProvider->ConsumeAmmo();
	}

	// 2. 카메라로부터 라인 트레이스
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
	if (UWorld* World = GetWorld())
	{
		bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		if (bHit && HitResult.GetActor())
		{
			CombatComponent->Server_ProcessHit(HitResult);
		}
	}
	
	// 3. Camera Shake
	UVGWeaponDataAsset* Data = CombatComponent->GetCurrentCombatData();
	if (Data && Data->FireCameraShake)
	{
		PlayerController->ClientStartCameraShake(Data->FireCameraShake);
	}
}