#include "Equipment/VGThrowableWeapon.h"

#include "Net/UnrealNetwork.h"

AVGThrowableWeapon::AVGThrowableWeapon()
{
	CurrentAmmo = 1;
}

bool AVGThrowableWeapon::HasAmmo() const
{
	return CurrentAmmo > 0;
}

void AVGThrowableWeapon::ConsumeAmmo()
{
	if (CurrentAmmo > 0)
	{
		CurrentAmmo--;
		
		if (CurrentAmmo <= 0 && HasAuthority())
		{
			Destroy();
		}
	}
}

int32 AVGThrowableWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

void AVGThrowableWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVGThrowableWeapon, CurrentAmmo);
}
