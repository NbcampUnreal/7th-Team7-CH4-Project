#include "Equipment/VGShield.h"

AVGShield::AVGShield()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = ShieldMesh;
	ShieldMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ShieldMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}