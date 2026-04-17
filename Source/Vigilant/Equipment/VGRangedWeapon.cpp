#include "Equipment/VGRangedWeapon.h"
#include "Net/UnrealNetwork.h"

bool AVGRangedWeapon::HasAmmo() const
{
	return CurrentAmmo > 0;
}

void AVGRangedWeapon::ConsumeAmmo()
{
	if (CurrentAmmo > 0)
	{
		CurrentAmmo--;
	}
}

int32 AVGRangedWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

void AVGRangedWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVGRangedWeapon, CurrentAmmo);
}
