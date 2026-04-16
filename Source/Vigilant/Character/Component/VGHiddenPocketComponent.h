// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VGHiddenPocketComponent.generated.h"

class AVGEquippableActor;

// UI 연동용 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPocketItemStashedSignature, UVGEquipmentDataAsset*, EquipmentData, UMeshComponent*, EquippedMesh);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPocketItemDroppedSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VIGILANT_API UVGHiddenPocketComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	UVGHiddenPocketComponent();
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// 현재 주머니에 숨겨둔 아이템
	UPROPERTY(ReplicatedUsing = OnRep_HiddenItem, VisibleAnywhere, BlueprintReadOnly, Category = "Pocket")
	TObjectPtr<AVGEquippableActor> HiddenItem;

	// H키를 눌렀을 때 호출될 메인 액션 함수 (줍기/버리기 토글)
	void TogglePocket();

	// 클라이언트 -> 서버 : 아이템 숨기기 요청
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Pocket|Action")
	void Server_StashItem(AVGEquippableActor* ItemToStash);

	// 클라이언트 -> 서버 : 숨긴 아이템 버리기 요청
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Pocket|Action")
	void Server_DropHiddenItem();

	// UI 바인드용 변수
	UPROPERTY(BlueprintAssignable, Category = "Pocket|Events")
	FOnPocketItemStashedSignature OnPocketItemStashed;

	UPROPERTY(BlueprintAssignable, Category = "Pocket|Events")
	FOnPocketItemDroppedSignature OnPocketItemDropped;

protected:
	UFUNCTION()
	void OnRep_HiddenItem(AVGEquippableActor* OldItem);

	// 아이템의 물리, 충돌, 가시성을 켜고 끄는 핵심 유틸리티
	void HandlePocketState(AVGEquippableActor* Item, bool bIsHiding);
};
