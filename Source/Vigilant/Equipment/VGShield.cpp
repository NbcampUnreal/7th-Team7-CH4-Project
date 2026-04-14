#include "Equipment/VGShield.h"

AVGShield::AVGShield()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	RootComponent = ShieldMesh;
	ShieldMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}
