#pragma once

#include "CoreMinimal.h"
#include "Component/VGEquipmentComponent.h"
#include "GameFramework/Character.h"
#include "VGCharacterBase.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UVGEquipmentComponent;

struct FInputActionValue;

UCLASS()
class VIGILANT_API AVGCharacterBase : public ACharacter
{
	GENERATED_BODY()

	// Components
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

protected:
	// Camera Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float DefaultCameraDistance = 400.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float MinCameraDistance = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float MaxCameraDistance = 800.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraZoomSpeed = 50.0f;

	// Walk & Sprint Speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float NormalSpeed = 600.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 900.0f;

	// Functions
public:
	AVGCharacterBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UVGEquipmentComponent> EquipmentComponent;
protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Input Handlers
	void Move(const FInputActionValue& Value);
	void StartJump(const FInputActionValue& Value);
	void StopJump(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);
	void CameraZoom(const FInputActionValue& Value);
	
	// 상호작용 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;
	// 아이템 버리기 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DropAction;
	// 슬롯 선택(1, 2) 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SlotSelectAction;
	// 현재 활성화된 슬롯을 기억할 변수 (기본값: 오른손)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	EVGEquipmentSlot ActiveEquipmentSlot = EVGEquipmentSlot::RightHand;
	// 상호작용 실행 함수
	void Interact();
	// 버리기 실행 함수
	void DropItem();
	// 슬롯 선택 실행 함수
	void SelectSlot(const FInputActionValue& Value);
	
	UFUNCTION(Server, Reliable)
	void ServerRPCSetSprinting(bool bIsSprinting);
};
