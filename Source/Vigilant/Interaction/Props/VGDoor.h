// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/VGInteractableActorBase.h"
#include "VGDoor.generated.h"

UCLASS()
class VIGILANT_API AVGDoor : public AVGInteractableActorBase
{
	GENERATED_BODY()

public:
	AVGDoor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// OnInteractWith 제거 — BP에서 처리

	UPROPERTY(ReplicatedUsing = OnRep_bIsLeftOpen, BlueprintReadWrite, Category = "VG|Door")
	bool bIsLeftOpen = false;

	UPROPERTY(ReplicatedUsing = OnRep_bIsRightOpen, BlueprintReadWrite, Category = "VG|Door")
	bool bIsRightOpen = false;
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "VG|Door")
	void K2_UpdateLeftDoorVisual();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "VG|Door")
	void K2_UpdateRightDoorVisual();
	
	UFUNCTION()
	void OnRep_bIsLeftOpen();
	
	UFUNCTION()
	void OnRep_bIsRightOpen();
};