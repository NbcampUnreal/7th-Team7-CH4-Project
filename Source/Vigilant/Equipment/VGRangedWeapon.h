#pragma once

#include "CoreMinimal.h"
#include "Combat/VGAmmoProviderInterface.h"
#include "Equipment/VGWeapon.h"
#include "VGRangedWeapon.generated.h"

UCLASS()
class VIGILANT_API AVGRangedWeapon : public AVGWeapon, public IVGAmmoProviderInterface
{
	GENERATED_BODY()

public:
	// IVGAmmoProviderInterface 구현
	virtual bool HasAmmo() const override;
	virtual void ConsumeAmmo() override;
	virtual int32 GetCurrentAmmo() const override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 20;
};
