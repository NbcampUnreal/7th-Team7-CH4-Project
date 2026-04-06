// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/VGWeapon.h"

AVGWeapon::AVGWeapon()
{
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = WeaponMesh;
	
	WeaponMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}
