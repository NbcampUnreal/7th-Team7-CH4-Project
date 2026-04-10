// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "VGBossSkillComponent.generated.h"

class UVGBossDataAsset;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VIGILANT_API UVGBossSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVGBossSkillComponent();

protected:
	virtual void BeginPlay() override;

public:    
	// Q 스킬
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Skill")
	void ExecuteSkill_Q();

	// E 스킬
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Skill")
	void ExecuteSkill_E();
	
protected:
	// 서버에게 스킬 사용을 요청 (쿨타임, 태그 검사)
	UFUNCTION(Server, Reliable)
	void Server_ExecuteSkill_Q();

	UFUNCTION(Server, Reliable)
	void Server_ExecuteSkill_E();

	// 서버가 모든 클라이언트에게 모션 재생 명령
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteSkill_Q();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteSkill_E();
	
	// AnimNotify에서 호출될 광역 공격 함수
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Skill|Roar")
	void ExecuteRoarAoE();
	
	// 착지 시 애니메이션 블루프린트에서 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Skill|Leap")
	void ExecuteLeapImpact();

public:
	// 데이터 에셋 참조 (블루프린트에서 보스 데이터 에셋 할당)
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Data")
	UVGBossDataAsset* BossDataAsset;
	
private:
	// 보스의 현재 상태 태그 바구니
	FGameplayTagContainer ActiveStateTags;

	// 타이머 핸들
	FTimerHandle TimerHandle_Q;
	FTimerHandle TimerHandle_E;

	// 내부 호출용 함수
	void ResetCooldown_Q();
	void ResetCooldown_E();

	bool bIsDashing = false;
	
	UFUNCTION()
	void OnSkillMontageEnded(class UAnimMontage* Montage, bool bInterrupted);
	
};
