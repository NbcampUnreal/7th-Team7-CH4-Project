#include "Combat/VGProjectileExecution.h"

#include "VGAmmoProviderInterface.h"
#include "Character/Component/VGCombatComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

void UVGProjectileExecution::StartAttack()
{
	Super::StartAttack();
	
	if (!CombatComponent.IsValid() || !ProjectileClass)
	{
		return;
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(CombatComponent->GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}
	
	UMeshComponent* TraceMesh = CombatComponent->GetActiveTraceMesh();
	if (!TraceMesh)
	{
		return;
	}
	
	// 1. 로컬 탄약 확인
	IVGAmmoProviderInterface* AmmoProvider = Cast<IVGAmmoProviderInterface>(TraceMesh->GetOwner());
	if (AmmoProvider)
	{
		if (!AmmoProvider->HasAmmo())
		{
			return;
		}
	}
	
	// 2. 카메라로부터 트레이스
	APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PlayerController)
	{
		return;
	}
	
	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	
	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * AimTraceDistance);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	
	FHitResult HitResult;
	FVector TargetLocation;
	
	UWorld* World = GetWorld();
	if (World && World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		TargetLocation = HitResult.ImpactPoint;
	}
	else
	{
		TargetLocation = TraceEnd;
	}
	
	// 3. Muzzle에서 대상까지의 Rotation 찾기
	FVector MuzzleLocation = TraceMesh->GetSocketLocation(MuzzleSocketName);
	FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, TargetLocation);
	
	// 4. 서버로 전송
	CombatComponent->Server_SpawnProjectile(ProjectileClass, MuzzleLocation, SpawnRotation);
	
	AmmoProvider->ConsumeAmmo();
}
