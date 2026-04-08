// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "VGMissionGimmickAltar.generated.h"

class AVGMissionItemBase;
class AVGMissionItemCarry;

USTRUCT(BlueprintType)
struct FVGAltarPlacementSlot
{
	GENERATED_BODY()

	// 이 슬롯에 필요한 아이템 타입
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag RequiredItemTypeTag;

	// 아이템이 부착될 소켓 이름 (없으면 액터 원점 기준 오프셋 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName AttachSocketName = NAME_None;

	// 소켓이 없을 때 사용할 상대 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector AttachOffset = FVector::ZeroVector;

	// 런타임: 현재 배치된 아이템
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<AVGMissionItemCarry> PlacedItem = nullptr;

	bool IsOccupied() const { return PlacedItem != nullptr; }
};

UCLASS()
class VIGILANT_API AVGMissionGimmickAltar : public AVGMissionGimmickBase
{
	GENERATED_BODY()

public:
	AVGMissionGimmickAltar();

	bool HasMatchingItemInHands(UVGEquipmentComponent* EquipComp, FGameplayTag GameplayTag) const;
	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const override;
	virtual void OnInteractWith(AVGCharacterBase* Interactor) override;
	
private:
	// 슬롯에 아이템 배치 시도
	bool TryPlaceItemToSlot(UVGEquipmentComponent* EquipComp, FVGAltarPlacementSlot& Slot);

	bool AreAllSlotsFilled();
protected:
	// Altar 슬롯 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Altar")
	TArray<FVGAltarPlacementSlot> PlacementSlots;
};
