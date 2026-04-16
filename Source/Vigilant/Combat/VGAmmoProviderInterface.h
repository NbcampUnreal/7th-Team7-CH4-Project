#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VGAmmoProviderInterface.generated.h"

UINTERFACE(MinimalAPI)
class UVGAmmoProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class VIGILANT_API IVGAmmoProviderInterface
{
	GENERATED_BODY()

public:
	virtual bool HasAmmo() const = 0;
	virtual void ConsumeAmmo() = 0;
	virtual int32 GetCurrentAmmo() const = 0;
};
