#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interaction/VGInteractableActorBase.h"
#include "Mission/VGMissionObjectInterface.h"
#include "VGMissionItemBase.generated.h"

class AVGMissionBase;
class AVGCharacterBase;

// Gimmick 상태 변경 시 어떤 기믹의 상태가 외부에 변했는지 알리기
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnItemStateChanged,
	AVGMissionItemBase*, Item,  // 어떤 아이템인지
	FGameplayTag, NewStateTag);

UCLASS(Abstract)
class VIGILANT_API AVGMissionItemBase : public AVGInteractableActorBase, public IVGMissionObjectInterface
{
	GENERATED_BODY()

public:
	AVGMissionItemBase();
	
	UFUNCTION(BlueprintCallable)
	AVGCharacterBase* GetCarrier() const { return Carrier; }
	
	virtual void GetLifetimeReplicatedProps(
			TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	bool IsCarried() const { return Carrier != nullptr; }

	virtual FGameplayTag GetStateTag() override { return ItemStateTag; }
	virtual void SetStateTag(FGameplayTag NewStateTag) override;
	
protected:
	// IVGInteractable 구현
	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const;
	virtual void OnInteractWith(AVGCharacterBase* Interactor);

	// 줍기 — 서버 전용
	virtual void OnPickedUp(AVGCharacterBase* NewCarrier);

	// 내려놓기 — 서버 전용
	virtual void OnDropped();

	UFUNCTION()
	virtual void OnRep_Carrier();
	
	UFUNCTION()
	virtual void OnRep_ItemStateTag();
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnItemStateChanged OnItemStateChanged;
	
protected:
	// 조건 충족 시 보고할 대상 미션 — 에디터에서 지정
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<AVGMissionBase> OwnerMission;

	// 현재 이 아이템을 들고 있는 캐릭터
	// Replicated: 아이템이 캐릭터를 따라 움직이는 시각 처리를 모든 클라이언트에 동기화
	UPROPERTY(ReplicatedUsing = OnRep_Carrier)
	TObjectPtr<AVGCharacterBase> Carrier;
	
	// 아이템 현재 상태 태그
	UPROPERTY(ReplicatedUsing = OnRep_ItemStateTag)
	FGameplayTag ItemStateTag;
};