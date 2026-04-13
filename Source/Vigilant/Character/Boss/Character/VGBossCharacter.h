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
	
private:
	void AddBossMappingContext(AController* InController);
	
protected:
	virtual void BeginPlay() override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void PawnClientRestart() override;
	
	virtual void Move(const FInputActionValue& Value) override;
	virtual void StartSprint(const FInputActionValue& Value) override;
	
	void Input_SkillQ(const FInputActionValue& Value);
	void Input_SkillE(const FInputActionValue& Value);
	
	// 보스 데이터 에셋 (에디터에서 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TObjectPtr<const UVGBossDataAsset> BossData;
	
	// 보스의 현재 체력
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BossStats")
	float CurrentHealth = 0.f;
	
	// 최종 스탯을 전달받아 세팅하는 함수
	UFUNCTION(BlueprintCallable, Category = "BossStats")
	void InitializeBossStats(float InCalculatedHealth, float InCalculatedDamage);
};
