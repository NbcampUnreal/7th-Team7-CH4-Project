// Fill out your copyright notice in the Description page of Project Settings.


#include "VGStatComponent.h"


UVGStatComponent::UVGStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UVGStatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
}


void UVGStatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

