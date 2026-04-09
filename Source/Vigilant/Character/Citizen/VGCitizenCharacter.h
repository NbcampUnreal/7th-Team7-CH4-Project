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


UCLASS()
class VIGILANT_API AVGCitizenCharacter :
	public AVGCharacterBase


{
	GENERATED_BODY()

public:
	AVGCitizenCharacter();
	void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UVGEquipmentComponent> EquipmentComponent;

protected:
	virtual void PawnClientRestart() override;
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

	// 캐릭터의 이동 상태(걷기, 낙하 등)가 변할 때마다 엔진이 호출해 주는 함수
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	// 상호작용 실행 함수
	void Interact();
	// 버리기 실행 함수
	void DropItem();
	// 슬롯 선택 실행 함수
	void SelectSlot(const FInputActionValue& Value);

	//base의 무브 함수 재정의
	virtual void Move(const FInputActionValue& Value) override;

	void Dodge();

	UFUNCTION(Server, Reliable)
	void Server_Dodge(FVector Direction);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Dodge();
	void PerformDodgeAction(const FVector& Direction);


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
	
	UFUNCTION()
	void HandleItemEquipped(EVGEquipmentSlot Slot, AVGEquippableActor* EquippedItem);
};
