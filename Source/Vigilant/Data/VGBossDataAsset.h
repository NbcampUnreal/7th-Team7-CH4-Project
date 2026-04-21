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
	float AttackRadius = 500.0f;
	
	// 이동 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float BossNormalSpeed = 400.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float BossSprintSpeed = 600.0f;
	
	// --- 스킬 데이터 ---
    
	// Q 스킬 (포효)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Q_Roar")
	class UAnimMontage* SkillMontage_Q;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Q_Roar")
	float CooldownTime_Q = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Q_Roar")
	float RoarBaseDamage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Q_Roar")
	float RoarRadius = 500.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Q_Roar")
	UParticleSystem* RoarEffect; // Q 스킬 이펙트

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Q_Roar")
	USoundBase* RoarSound; // Q 스킬 사운드

	// E 스킬 (점프 찍기)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|E_Leap")
	UAnimMontage* SkillMontage_E;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|E_Leap")
	float LeapDamage = 50.f;   

	UPROPERTY(EditDefaultsOnly, Category = "Skill|E_Leap")
	float LeapRadius = 500.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|E_Leap")
	float CooldownTime_E = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|E_Leap")
	UParticleSystem* LeapEffect; // E 스킬 이펙트

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|E_Leap")
	USoundBase* LeapSound; // E 스킬 사운드
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	UAnimMontage* DeathMontage;
	// (추후 확장) 페이즈별 강화 수치나 이펙트 등을 여기에 추가
};
