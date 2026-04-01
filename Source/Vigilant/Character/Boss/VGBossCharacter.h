// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/VGCharacterBase.h"
#include "VGBossCharacter.generated.h"

class UVGBossDataAsset;
class UAnimMontage;
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
	
	// 클라이언트(또는 서버)가 공격을 실행할 때 호출하는 서버 RPC (데미지 판정용)
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BossAction")
	void Server_PerformAttack();

	// 서버가 모든 클라이언트에게 이펙트와 애니메이션 재생을 지시하는 멀티캐스트 RPC (시각 효과용)
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticast_PlayAttackEffects();
protected:
	virtual void BeginPlay() override;
	
	// 보스 데이터 에셋 (에디터에서 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TObjectPtr<const UVGBossDataAsset> BossData;
	
	// 보스의 현재 체력
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BossStats")
	float CurrentHealth;
	
	// 보스의 공격력
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BossStats")
	float AttackDamage;
	
	// 보스의 광역 공격 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossStats")
	float AttackRadius;
	
	// 보스 공격 시 재생할 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossStats")
	TObjectPtr<UAnimMontage> AttackMontage;
	
	// 최종 스탯을 전달받아 세팅하는 함수
	UFUNCTION(BlueprintCallable, Category = "BossStats")
	void InitializeBossStats(float InCalculatedHealth, float InCalculatedDamage);
};
