// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interaction/VGInteractableActorBase.h"
#include "VGMissionGimmickBase.generated.h"

class AVGMissionBase;

UCLASS()
class VIGILANT_API AVGMissionGimmickBase : public AVGInteractableActorBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVGMissionGimmickBase();
	
	// 상속
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Todo IVGInteractable 구현
	
protected:
	// IVGInteractable 구현
	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const;
	virtual void OnInteractWith(AVGCharacterBase* Interactor);
	
	// 조건 충족 시 자식 클래스에서 호출 -> OwnerMission에 보고
	virtual void ReportConditionMet();
	
	void SetGimmickState(FGameplayTag NewStateTag);
	
	UFUNCTION()
	virtual void OnRep_GimmickStateTag();
	
protected:
	// 조건 충족 시 보고할 대상 미션 — 에디터에서 지정
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<AVGMissionBase> OwnerMission;
	
	// 기믹 현재 상태 태그
	UPROPERTY(ReplicatedUsing = OnRep_GimmickStateTag)
	FGameplayTag GimmickStateTag;
};
