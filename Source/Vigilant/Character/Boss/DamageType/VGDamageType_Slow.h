// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "VGDamageType_Slow.generated.h"

/**
 *  이동 속도 감소(슬로우) 디버프를 전달하기 위한 데미지 타입
 */
UCLASS()
class VIGILANT_API UVGDamageType_Slow : public UDamageType
{
	GENERATED_BODY()
	
public:
	// 데미지 타입 자체가 슬로우 수치를 기억하게 만듭니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slow")
	float SlowMultiplier = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slow")
	float SlowDuration = 3.0f;
};
