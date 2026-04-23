#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "VGEquipmentDataAsset.generated.h"

class UAnimInstance;

UENUM(BlueprintType)
enum class EVGEquipRules : uint8
{
	RightHandOnly UMETA(DisplayName = "Right Hand Only"), // 오른손 전용 (한손검)
	LeftHandOnly UMETA(DisplayName = "Left Hand Only"), // 왼손 전용 (방패)
	EitherHand UMETA(DisplayName = "Either Hand"), // 양손 사용 가능 (미션 아이템)
	BothHands UMETA(DisplayName = "Both Hands"), // 양손 전용 (양손검)
};

/**
 * 장착 가능한 모든 아이템의 공통 데이터를 담는 기반 Data Asset.
 *
 * 무기, 방패 등 장착 아이템 Data Asset은 이 클래스를 상속받아 구현합니다.
 * Equip 규칙, 장착 소켓 매핑, 장착 시 적용할 애님 레이어를 정의합니다.
 */
UCLASS(BlueprintType)
class VIGILANT_API UVGEquipmentDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|UI")
	TObjectPtr<UTexture2D> ItemIcon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	EVGEquipRules EquipRule;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FGameplayTagContainer GrantedEquipmentTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Sockets")
	FName RightHandSocketName = TEXT("RightHandSocket");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Sockets")
	FName LeftHandSocketName = TEXT("LeftHandSocket");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<UAnimInstance> EquippedAnimLayer;
};