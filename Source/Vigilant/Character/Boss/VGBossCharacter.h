// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/VGCharacterBase.h"
#include "VGBossCharacter.generated.h"

class UVGBossDataAsset;
class UAnimMontage;
class UVGCombatComponent;

UCLASS()
class VIGILANT_API AVGBossCharacter : public AVGCharacterBase
{
	GENERATED_BODY()
	
public:
	AVGBossCharacter();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	
	// 보스 데이터 에셋 (에디터에서 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TObjectPtr<const UVGBossDataAsset> BossData;
	
	// 보스의 현재 체력
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BossStats")
	float CurrentHealth;
	
	// 최종 스탯을 전달받아 세팅하는 함수
	UFUNCTION(BlueprintCallable, Category = "BossStats")
	void InitializeBossStats(float InCalculatedHealth, float InCalculatedDamage);
};
