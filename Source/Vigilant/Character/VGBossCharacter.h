// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/VGCharacterBase.h"
#include "VGBossCharacter.generated.h"

/**
 * 
 */
UCLASS()
class VIGILANT_API AVGBossCharacter : public AVGCharacterBase
{
	GENERATED_BODY()
	
public:
	AVGBossCharacter();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// 보스의 현재 체력
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BossStats")
	float CurrentHealth;
	
	// 보스의 공격력
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BossStats")
	float AttackDamage;
};
