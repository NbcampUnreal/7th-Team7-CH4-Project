#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VGProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class VIGILANT_API AVGProjectile : public AActor
{
	GENERATED_BODY()

public:
	AVGProjectile();

	void InitializeProjectile(float InDamage);

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                             FVector NormalImpulse, const FHitResult& Hit);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
private:
	float ProjectileDamage = 0.0f;
};
