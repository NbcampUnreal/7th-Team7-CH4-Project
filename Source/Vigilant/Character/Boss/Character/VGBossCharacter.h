// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/VGCharacterBase.h"
#include "VGBossCharacter.generated.h"

class UVGBossDataAsset;
class UVGBossSkillComponent;

UCLASS()
class VIGILANT_API AVGBossCharacter : public AVGCharacterBase
{
	GENERATED_BODY()
	
public:
	AVGBossCharacter();
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	// 보스 스킬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vigilant|Component")
	class UVGBossSkillComponent* SkillComponent;
	
	// 에디터에서 할당할 보스 IMC
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vigilant|Input")
	class UInputMappingContext* BossMappingContext;

	// 에디터에서 할당할 보스 스킬 IA
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vigilant|Input")
	class UInputAction* SkillAction_Q;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vigilant|Input")
	class UInputAction* SkillAction_E;
	
	// 미션과 투표 결과에 따라 달라질 스택 적용할 용도
	void ApplyNerfAndInitStat(float NerfRate);

protected:
	virtual void BeginPlay() override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void PawnClientRestart() override;
	
	virtual void Move(const FInputActionValue& Value) override;
	virtual void StartSprint(const FInputActionValue& Value) override;
	
	void Input_SkillQ(const FInputActionValue& Value);
	void Input_SkillE(const FInputActionValue& Value);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Die();
	UFUNCTION()
	void Die(AController* Killer);
	
	// 보스 데이터 에셋 (에디터에서 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vigilant|Data")
	TObjectPtr<const UVGBossDataAsset> BossData;
	
	// 최종 스탯을 전달받아 세팅하는 함수
	UFUNCTION(BlueprintCallable, Category = "BossStats")
	void InitializeBossStats(float InCalculatedHealth, float InCalculatedDamage);
		
private:
	void AddBossMappingContext(AController* InController);
};
