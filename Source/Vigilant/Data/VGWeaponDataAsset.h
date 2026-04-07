#pragma once

#include "CoreMinimal.h"
#include "VGEquipmentDataAsset.h"
#include "VGWeaponDataAsset.generated.h"

/**
 * 무기의 공격 관련 데이터를 담는 Data Asset.
 *
 * UVGEquipmentDataAsset을 상속받으며, Damage / AttackSpeed / Montage 등
 * 근접 공격에 필요한 모든 데이터를 포함합니다.
 *
 * 플레이어의 맨손 공격 및 Boss 기본 공격에도 재사용되며,
 * 해당 경우 DropPriority 같은 Equipment 전용 필드는 무시됩니다.
 *
 * @note 원거리 무기가 필요하다면 이 클래스를 상속해 UVGRangedWeaponDataAsset을 만들고
 *       ProjectileClass 프로퍼티를 추가하면 됩니다.
 * @note 방패가 필요하다면 UVGEquipmentDataAsset을 직접 상속해 UVGShieldDataAsset을 만들면 됩니다.
 */
UCLASS(BlueprintType)
class VIGILANT_API UVGWeaponDataAsset : public UVGEquipmentDataAsset
{
	GENERATED_BODY()

public:
	// [Fix] 초기값 0.f 명시 — 에디터에서 미설정 시 쓰레기 값 방지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float BaseDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats", meta = (ClampMin = "0.1"))
	float AttackSpeed = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|HitDetection")
	TArray<FName> HitboxSocketNames;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> LightAttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;
};
