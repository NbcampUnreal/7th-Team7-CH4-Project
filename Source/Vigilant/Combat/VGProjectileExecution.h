#pragma once

#include "CoreMinimal.h"
#include "Combat/VGAttackExecution.h"
#include "VGProjectileExecution.generated.h"

UCLASS()
class VIGILANT_API UVGProjectileExecution : public UVGAttackExecution
{
	GENERATED_BODY()

public:
	virtual void StartAttack() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AActor> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	FName MuzzleSocketName = TEXT("Muzzle");
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float AimTraceDistance = 10000.0f;
};
