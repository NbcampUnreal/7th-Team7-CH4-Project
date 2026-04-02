#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "VGEquipmentDataAsset.generated.h"

class UAnimInstance;

/**
 * 장착 가능한 모든 아이템의 공통 데이터를 담는 기반 Data Asset.
 *
 * 무기, 방패 등 장착 아이템 Data Asset은 이 클래스를 상속받아 구현합니다.
 * 드롭 우선순위, 장착 슬롯/소켓 매핑, 장착 시 적용할 애님 레이어를 정의합니다.
 */
UCLASS(BlueprintType)
class VIGILANT_API UVGEquipmentDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	/**
	 * 아이템이 장착될 수 있는 슬롯과 소켓의 매핑 테이블.
	 *
	 * - Key   : 아이템이 지원하는 장착 슬롯 (Gameplay Tag)
	 * - Value : 해당 슬롯에서 사용할 스켈레탈 메시 소켓 이름
	 *
	 * @par 예시
	 * 검 (오른손 전용)  : { Equipment.Slot.RightHand, "Sword_R_Socket" }
	 * 열쇠 (양손 가능)  : { Equipment.Slot.RightHand, "Key_R_Socket" },
	 *                       { Equipment.Slot.LeftHand,  "Key_L_Socket"  }
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TMap<FGameplayTag, FName> SupportedSlotsAndSockets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	int32 DropPriority;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<UAnimInstance> EquippedAnimLayer;
	
};