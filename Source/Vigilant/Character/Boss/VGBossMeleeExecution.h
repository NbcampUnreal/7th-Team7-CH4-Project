// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/VGMeleeExecution.h"
#include "VGBossMeleeExecution.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class VIGILANT_API UVGBossMeleeExecution : public UVGMeleeExecution
{
	GENERATED_BODY()
	
public:
	virtual void StartAttack() override;
	
	virtual void TickAttack() override;
	
private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> BossHitActorsThisSwing;
	UPROPERTY(Transient)
	TMap<FName, FVector> BossPrevSockets;

	bool bIsFirstTick;
};
