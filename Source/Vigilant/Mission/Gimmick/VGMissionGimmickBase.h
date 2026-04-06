// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interaction/VGInteractableActorBase.h"
#include "Mission/VGMissionObjectInterface.h"
#include "VGMissionGimmickBase.generated.h"

class AVGMissionBase;

// Gimmick 상태 변경 시 어떤 기믹의 상태가 외부에 변했는지 알리기
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnGimmickStateChanged,
	AVGMissionGimmickBase*, Gimmick,  // 어떤 기믹인지
	FGameplayTag, NewStateTag); 

UCLASS()
class VIGILANT_API AVGMissionGimmickBase : public AVGInteractableActorBase, public IVGMissionObjectInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVGMissionGimmickBase();
	
	// 상속
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 미션 실패 시 리셋
	UFUNCTION()
	virtual void ResetGimmickState();
	
	FGameplayTag GetStateTag() { return GimmickStateTag; }
	void SetStateTag(FGameplayTag NewStateTag);
	
	void SetOwnerMission(AVGMissionBase* InOwnerMission);
	
	// 조건 충족 시 자식 클래스에서 호출 -> OwnerMission에 보고
	virtual void ReportConditionMet();
	
protected:
	// IVGInteractable 구현
	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const;
	virtual void OnInteractWith(AVGCharacterBase* Interactor);
	
	
	UFUNCTION()
	virtual void OnRep_GimmickStateTag();
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnGimmickStateChanged OnGimmickStateChanged;
	
protected:
	// 오브젝트 매쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	// 조건 충족 시 보고할 대상 미션 — 에디터에서 지정
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<AVGMissionBase> OwnerMission;
	
	// 기믹 현재 타입 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|Gimmick")
	FGameplayTag GimmickTypeTag;
	
	// 기믹 현재 상태 태그
	UPROPERTY(ReplicatedUsing = OnRep_GimmickStateTag)
	FGameplayTag GimmickStateTag;
	
private:
	UMaterialInstanceDynamic* DynamicMaterialInstance;
};
