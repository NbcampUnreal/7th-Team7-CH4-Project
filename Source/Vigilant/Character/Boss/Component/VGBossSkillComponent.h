// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "VGBossSkillComponent.generated.h"


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
	// 2. 서버에게 스킬 사용을 요청 (쿨타임, 태그 검사)
	UFUNCTION(Server, Reliable)
	void Server_ExecuteSkill_Q();

	UFUNCTION(Server, Reliable)
	void Server_ExecuteSkill_E();

	// 3. 서버가 모든 클라이언트에게 모션 재생 명령
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteSkill_Q();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteSkill_E();
	
public:
	// 애니메이션 몽타주 변수
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Skill")
	class UAnimMontage* SkillMontage_Q;

	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Skill")
	class UAnimMontage* SkillMontage_E;
	
	// 블루프린트에서 설정할 쿨타임 수치
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Skill|Cooldown")
	float CooldownTime_Q = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Skill|Cooldown")
	float CooldownTime_E = 8.0f;
	
private:
	// 보스의 현재 상태 태그 바구니
	FGameplayTagContainer ActiveStateTags;

	// 타이머 핸들
	FTimerHandle TimerHandle_Q;
	FTimerHandle TimerHandle_E;

	// 내부 호출용 함수
	void ResetCooldown_Q();
	void ResetCooldown_E();

	UFUNCTION()
	void OnSkillMontageEnded(class UAnimMontage* Montage, bool bInterrupted);
};
