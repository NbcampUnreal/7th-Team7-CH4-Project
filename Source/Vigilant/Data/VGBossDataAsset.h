// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VGBossDataAsset.generated.h"

class UAnimMontage;

/**
 *  보스 캐릭터의 기본 스탯과 액션 에셋을 관리하는 데이터 에셋
 */
UCLASS()
class VIGILANT_API UVGBossDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 생존 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float BaseHealth = 5000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float BaseDefense = 50.0f;

	// 공격 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float BaseDamage = 50.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackRadius = 500.0f;
	
	// 이동 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float BossNormalSpeed = 400.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float BossSprintSpeed = 600.0f;
	
	// (추후 확장) 페이즈별 강화 수치나 이펙트 등을 여기에 추가
};
