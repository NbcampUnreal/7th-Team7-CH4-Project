// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "Character/VGCharacterBase.h"
#include "Character/CharacterInterface/VGCharacterGameplayTagEditor.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "VGCitizenCharacter.generated.h"

class UVGEquipmentComponent;
class UInputAction;
struct FInputActionValue;

// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentSlotChangedSignature, EVGEquipmentSlot, NewActiveSlot);

UCLASS()
class VIGILANT_API AVGCitizenCharacter : 
public AVGCharacterBase, 
public IVGCharacterGameplayTagEditor,
public IGameplayTagAssetInterface

{
	GENERATED_BODY()
	

	
	
#pragma region Interfaces Func
public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual void AddGameplayTag(FGameplayTag TagToAdd) override;
	virtual void RemoveGameplayTag(FGameplayTag TagToRemove) override;
#pragma endregion 김형백
	
public:
	AVGCitizenCharacter();
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UVGEquipmentComponent> EquipmentComponent;
	
	// UI(블루프린트)에서 이벤트로 끌어다 쓸 수 있는 델리게이트 변수
	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnEquipmentSlotChangedSignature OnEquipmentSlotChanged;
	
protected:
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// 상호작용 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;
	// 아이템 버리기 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DropAction;
	// 슬롯 선택(1, 2) 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SlotSelectAction;
	//구르기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DodgeAction;
	// 현재 활성화된 슬롯을 기억할 변수 (기본값: 오른손)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	EVGEquipmentSlot ActiveEquipmentSlot = EVGEquipmentSlot::RightHand;
	
	// 상호작용 실행 함수
	void Interact();
	// 버리기 실행 함수
	void DropItem();
	// 슬롯 선택 실행 함수
	void SelectSlot(const FInputActionValue& Value);
	
	//base의 무브 함수 재정의
	virtual void Move(const FInputActionValue& Value) override;
	
	void Dodge();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Force")
	float DodgeForce = 600.0f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Force")
	float DodgeZForce = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Force")
	float ModifyFriction;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dodge|Force")
	float OriginalFriction; // 기본값 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> DodgeAnimation;
	
	UFUNCTION()
	void OnMontageCompleted(UAnimMontage* Montage, bool bWasCancelled = false);
};
