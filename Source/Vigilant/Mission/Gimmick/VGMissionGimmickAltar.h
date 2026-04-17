// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGMissionGimmickBase.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "VGMissionGimmickAltar.generated.h"

class AVGMissionItemBase;
class AVGMissionItemCarry;
class UNiagaraSystem;
class UNiagaraComponent;
class UVGMissionItemDataAsset;

USTRUCT(BlueprintType)
struct FVGAltarPlacementSlot
{
	GENERATED_BODY()

	// 이 슬롯에 필요한 아이템의 데이터 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UVGMissionItemDataAsset> ItemDataAsset = nullptr;

	// 소켓이 없을 때 사용할 상대 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector AttachOffset = FVector::ZeroVector;
	
	// Hint Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> RequiredItemHintEffect = nullptr;
	
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

	virtual bool CanInteractWith(AActor* Interactor) const override;
	virtual void OnInteractWith(AActor* Interactor, const FTransform& InteractTransform) override;
	
protected:
	virtual void BeginPlay() override;
	void UpdateHintEffectVisibility();
	
private:
	// 슬롯에 아이템 배치 시도
	bool TryPlaceItemToSlot(UVGEquipmentComponent* EquipComp, FVGAltarPlacementSlot& Slot);

	bool AreAllSlotsFilled() const;

protected:
	// Altar 슬롯 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Altar")
	TArray<FVGAltarPlacementSlot> PlacementSlots;
	
	// 런타임에 BeginPlay에서 동적 생성되며 PlacementSlots와 인덱스가 1:1 대응됨 (힌트 이펙트가 없는 슬롯은 nullptr로 채움)
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNiagaraComponent>> HintEffectComponents;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Altar")
	float HintVisibleRange = 800.f;
	
	FTimerHandle HintVisibilityTimerHandle;
};
