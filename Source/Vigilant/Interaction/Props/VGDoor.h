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

	virtual  void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void OnInteractWith(AActor* Interactor, const FTransform& InteractTransform) override;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsOpen, BlueprintReadOnly, Category = "VG|Door")
	bool bIsOpen = false;
	
	UFUNCTION()
	void OnRep_IsOpen();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "VG|Door")
	void K2_OnDoorStateChanged(bool bNewIsOpen);
};
