#pragma once

#include "CoreMinimal.h"
#include "VGEquipmentDataAsset.h"
#include "VGWeaponDataAsset.generated.h"

class UVGAttackExecution;
class UNiagaraSystem;

/**
 * 무기의 공격 관련 데이터를 담는 Data Asset.
 *
 * UVGEquipmentDataAsset을 상속받으며, Damage / AttackSpeed / Montage 등
 * 근접 공격에 필요한 모든 데이터를 포함합니다.
 *
 * 플레이어의 맨손 공격 및 Boss 기본 공격에도 재사용되며,
 * 해당 경우 DropPriority 같은 Equipment 전용 필드는 무시됩니다.
 */
UCLASS(BlueprintType)
class VIGILANT_API UVGWeaponDataAsset : public UVGEquipmentDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float BaseDamage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float LightAttackDamage = 15.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float LightAttackStaminaCost = 10.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float HeavyAttackDamage = 30.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float HeavyAttackStaminaCost = 20.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Weapon|Movement")
	bool bFaceCameraDuringAttack = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats", meta = (ClampMin = "0.1"))
	float AttackSpeed = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float MaxAttackRange = 300.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|HitDetection")
	TArray<FName> HitboxSocketNames;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> LightAttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Weapon|Execution")
	TObjectPtr<UVGAttackExecution> AttackExecutionTemplate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<UNiagaraSystem> ImpactVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TSubclassOf<AActor> DummyActorClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX")
	TObjectPtr<UNiagaraSystem> MeleeTrailVFX;
};
