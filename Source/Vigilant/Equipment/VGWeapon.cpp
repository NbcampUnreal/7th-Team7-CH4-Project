#include "Equipment/VGWeapon.h"

AVGWeapon::AVGWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = WeaponMesh;
	WeaponMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}
