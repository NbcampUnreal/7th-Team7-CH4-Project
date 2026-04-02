// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "VGEquipmentComponent.generated.h"

class AVGEquippableActor;

// 장착 슬롯 구분 (왼손, 오른손, 양손)
UENUM(BlueprintType)
enum class EVGEquipmentSlot : uint8
{
	None UMETA(DisplayName = "None"),
	LeftHand UMETA(DisplayName = "Left Hand"),
	RightHand UMETA(DisplayName = "Right Hand"),
	BothHands UMETA(DisplayName = "Both Hands")
};

// 장비 타입 구분
UENUM(BlueprintType)
enum class EVGEquipmentType : uint8
{
	None UMETA(DisplayName = "None"),
	Weapon UMETA(DisplayName = "Weapon"),
	Shield UMETA(DisplayName = "Shield"),
	TwoHandedWeapon UMETA(DisplayName = "Two-Handed Weapon"),
	MissionItem UMETA(DisplayName = "Mission Item")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VIGILANT_API UVGEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVGEquipmentComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	// 양손 인벤토리 변수
	// 왼손 (방패 또는 미션 아이템)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Slots")
	TObjectPtr<AVGEquippableActor> LeftHandItem;

	// 오른손 (무기 또는 미션 아이템)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Slots")
	TObjectPtr<AVGEquippableActor> RightHandItem;
	
	// 현재 장착 상태를 나타내는 게임플레이 태그 컨테이너
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Tags")
	FGameplayTagContainer EquipmentTags;
	
	// 클라이언트가 서버에게 아이템 장착을 요청하는 함수
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Equipment|Action")
	void Server_EquipItem(AVGEquippableActor* ItemToEquip, EVGEquipmentType ItemType);
	
	// 클라이언트가 서버에게 아이템 버리기를 요청하는 함수
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Equipment|Action")
	void Server_DropItem(EVGEquipmentSlot SlotToDrop);
};
