#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "VGMissionGimmickChest.generated.h"

class AVGEquippableActor;
class UBGEquipmentComponent;

UCLASS()
class VIGILANT_API AVGMissionGimmickChest : public AVGMissionGimmickBase
{
	GENERATED_BODY()

public:
	AVGMissionGimmickChest();

	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const override;
	virtual void Server_Interact_Implementation(AVGCharacterBase* Interactor) override;

private:
	// 슬롯에서 필요한 아이템을 찾아 사용 처리
	// 성공하면 true 반환
	bool TryUseItemFromSlot(
		UVGEquipmentComponent* EquipComp,
		AVGEquippableActor* SlotItem,
		EVGEquipmentSlot Slot);

protected:
	// 열쇠 아이템 타입 태그 — 이 태그를 가진 아이템만 사용 가능
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Chest")
	FGameplayTag RequiredItemTypeTag;
};
