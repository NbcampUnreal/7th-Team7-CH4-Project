// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGEquipmentDataAsset.h"
#include "VGMissionItemDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class VIGILANT_API UVGMissionItemDataAsset : public UVGEquipmentDataAsset
{
	GENERATED_BODY()
	
public:
	// 아이템 표시 이름 — UI에서 활용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText ItemDisplayName;

	// 아이템 설명 — UI 툴팁 등
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText ItemDescription;

	// 아이템 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visual")
	TObjectPtr<UStaticMesh> ItemMesh;

	// 픽업 시 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Effect")
	TObjectPtr<UParticleSystem> PickupParticle;

	// 사용/배치 완료 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Effect")
	TObjectPtr<UParticleSystem> UsedParticle;

	// 아이템 타입 태그 (Mission.Item.Type.Key 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Tags")
	FGameplayTag ItemTypeTag;
};
