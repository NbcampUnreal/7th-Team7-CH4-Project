// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/VGCharacterBase.h"

// Sets default values
AVGCharacterBase::AVGCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVGCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVGCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AVGCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

