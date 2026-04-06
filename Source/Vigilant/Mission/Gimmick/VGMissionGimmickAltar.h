// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "VGMissionGimmickAltar.generated.h"

UCLASS()
class VIGILANT_API AVGMissionGimmickAltar : public AVGMissionGimmickBase
{
	GENERATED_BODY()

public:
	AVGMissionGimmickAltar();

	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const override;
	virtual void OnInteractWith(AVGCharacterBase* Interactor) override;
	
private:
	// 슬롯에서 필요한 아이템을 찾아 사용 처리
	// 성공하면 true 반환
	bool TryPlaceItemFromSlot(
		UVGEquipmentComponent* EquipComp,
		AVGEquippableActor* SlotItem,
		EVGEquipmentSlot Slot);

protected:
	// 올려놓을 수 있는 아이템 타입 태그
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Altar")
	FGameplayTag RequiredItemTypeTag;

	// 현재 올려진 아이템
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gimmick|Altar")
	TObjectPtr<AVGMissionItemBase> PlacedItem;
};
