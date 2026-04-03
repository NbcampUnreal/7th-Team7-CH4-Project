// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Mission/VGMissionObjectInterface.h"
#include "Interaction/VGInteractableActorBase.h"
#include "VGMissionGimmickBase.generated.h"

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
	
	virtual bool CanInteractWith(AVGCharacterBase* Interactor) const override;
	virtual void OnInteractWith(AVGCharacterBase* Interactor) override;
	
	// 미션 실패 시 리셋
	UFUNCTION()
	virtual void ResetGimmickState();
	
	FGameplayTag GetStateTag() { return GimmickStateTag; }
	void SetStateTag(FGameplayTag NewStateTag);
	
	void SetGimmickIndex(int32 NewGimmickIndex);
	int32 GetGimmickIndex() {return GimmickIndex;}

protected:
	UFUNCTION()
	virtual void OnRep_GimmickStateTag();
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnGimmickStateChanged OnGimmickStateChanged;
	
protected:
	// 오브젝트 매쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	// 기믹 현재 타입 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|Gimmick")
	FGameplayTag GimmickTypeTag;
	
	// 기믹 현재 상태 태그
	UPROPERTY(ReplicatedUsing = OnRep_GimmickStateTag)
	FGameplayTag GimmickStateTag;
	
	// BeginPlay에서 MissionBase가 자동 부여 — 에디터 지정 불필요
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission|Gimmick")
	int32 GimmickIndex = -1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|Effect")
	UParticleSystem* InactiveParticle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|Effect")
	UParticleSystem* ActiveParticle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission|Effect")
	UParticleSystem* CompleteParticle;
	
private:
	UMaterialInstanceDynamic* DynamicMaterialInstance;
};
