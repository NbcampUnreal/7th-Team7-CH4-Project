#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VGCharacterBase.generated.h"

class UCameraComponent;
class USpringArmComponent;

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
	
	UFUNCTION(Server, Reliable)
	void ServerRPCSetSprinting(bool bIsSprinting);
};
