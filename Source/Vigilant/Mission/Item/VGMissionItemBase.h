#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interaction/VGInteractableActorBase.h"
#include "Mission/VGMissionObjectInterface.h"
#include "Equipment/VGEquippableActor.h"
#include "VGMissionItemBase.generated.h"

class AVGMissionBase;
class AVGCharacterBase;
class UVGMissionItemDataAsset;

// Gimmick 상태 변경 시 어떤 기믹의 상태가 외부에 변했는지 알리기
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnItemStateChanged,
	AVGMissionItemBase*, Item,  // 어떤 아이템인지
	FGameplayTag, NewStateTag);

UCLASS(Abstract)
class VIGILANT_API AVGMissionItemBase : public AVGEquippableActor, public IVGMissionObjectInterface
{
	GENERATED_BODY()

public:
	AVGMissionItemBase();
	
	virtual void GetLifetimeReplicatedProps(
			TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	AVGCharacterBase* GetCarrier() const { return Carrier; }
	
	UFUNCTION(BlueprintCallable)
	bool IsCarried() const { return Carrier != nullptr; }

	// [Fix] IVGMissionObjectInterface — const 추가
	virtual FGameplayTag GetStateTag() const override { return ItemStateTag; }
	virtual void SetStateTag(FGameplayTag NewStateTag) override;
	
	// 상호작용 — 줍기 진입점
	UFUNCTION(BlueprintCallable)
	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const;

	UFUNCTION(BlueprintCallable)
	virtual void OnInteractWith(AVGCharacterBase* Interactor);
	
	// 내려놓기 — EquipComponent에서 호출
	virtual void OnDropped();
	
protected:
	// 줍기 — 서버 전용
	virtual void OnPickedUp(AVGCharacterBase* NewCarrier);

	UFUNCTION()
	virtual void OnRep_Carrier();
	
	UFUNCTION()
	virtual void OnRep_ItemStateTag();
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnItemStateChanged OnItemStateChanged;
	
	// DataAsset — 메시, 소켓 정보 등
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UVGMissionItemDataAsset> ItemDataAsset;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	// 현재 이 아이템을 들고 있는 캐릭터
	// Replicated: 아이템이 캐릭터를 따라 움직이는 시각 처리를 모든 클라이언트에 동기화
	UPROPERTY(ReplicatedUsing = OnRep_Carrier)
	TObjectPtr<AVGCharacterBase> Carrier;
	
	// 아이템 현재 상태 태그
	UPROPERTY(ReplicatedUsing = OnRep_ItemStateTag)
	FGameplayTag ItemStateTag;
};