#include "Combat/VGProjectile.h"

#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AVGProjectile::AVGProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(5.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->bReturnMaterialOnMove = true;
	RootComponent = CollisionComponent;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;

	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	InitialLifeSpan = 5.0f;
	
	TrailVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailVFX"));
	TrailVFXComponent->SetupAttachment(RootComponent);
	TrailVFXComponent->bAutoActivate = true;
}

void AVGProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (GetInstigator())
	{
		CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	}

	if (HasAuthority())
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &AVGProjectile::OnProjectileHit);
	}
}

void AVGProjectile::InitializeProjectile(float InDamage)
{
	ProjectileDamage = InDamage;
}

void AVGProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	if (ProjectileDamage > 0.0f)
	{
		UGameplayStatics::ApplyDamage(OtherActor, ProjectileDamage, GetInstigatorController(), GetInstigator(),
		                              nullptr);
	}
	
	if (TrailVFXComponent)
	{
		TrailVFXComponent->Deactivate();
		TrailVFXComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
	
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetVisibility(false);
	ProjectileMovement->StopMovementImmediately();

	Multicast_PlayImpactFeedback(Hit.ImpactPoint, Hit.ImpactNormal, OtherComp);
	SetLifeSpan(0.1f);
}

void AVGProjectile::Multicast_PlayImpactFeedback_Implementation(FVector ImpactPoint, FVector ImpactNormal,
                                                                UPrimitiveComponent* HitComponent)
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ImpactPoint);
	}

	if (ImpactVFX)
	{
		FRotator VFXRotation = ImpactNormal.Rotation();
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactVFX, ImpactPoint, VFXRotation);
	}

	if (DummyActorClass && HitComponent)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FRotator DummyRotation = ImpactNormal.Rotation();
		DummyRotation.Pitch -= 180.0f;


		UWorld* World = GetWorld();
		if (World)
		{
			AActor* Dummy = World->SpawnActor<AActor>(DummyActorClass, ImpactPoint, DummyRotation, SpawnParams);
			if (Dummy)
			{
				Dummy->AttachToComponent(HitComponent, FAttachmentTransformRules::KeepWorldTransform);
				Dummy->SetLifeSpan(10.0f);
			}
		}
	}
}
