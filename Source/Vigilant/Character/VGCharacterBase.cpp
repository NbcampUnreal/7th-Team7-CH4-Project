#include "Character/VGCharacterBase.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Component/VGCombatComponent.h"
#include "Core/VGPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Equipment/VGEquippableActor.h"
#include "DrawDebugHelpers.h"

AVGCharacterBase::AVGCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Configure Character Movement
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

	// Create the camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	CameraBoom->TargetArmLength = DefaultCameraDistance;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = false;
	CameraBoom->bEnableCameraRotationLag = true;

	// create the orbiting camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	// create the combat component
	CombatComponent = CreateDefaultSubobject<UVGCombatComponent>(TEXT("CombatComponent"));
}

void AVGCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AVGCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AVGPlayerController* PlayerController = Cast<AVGPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this,
				                          &AVGCharacterBase::Move);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this,
				                          &AVGCharacterBase::Look);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Started, this,
				                          &AVGCharacterBase::StartJump);
				EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Completed, this,
				                          &AVGCharacterBase::StopJump);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Started, this,
				                          &AVGCharacterBase::StartSprint);
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Completed, this,
				                          &AVGCharacterBase::StopSprint);
			}

			if (PlayerController->LightAttackAction)
			{
				EnhancedInput->BindAction(PlayerController->LightAttackAction, ETriggerEvent::Started, this,
				                          &AVGCharacterBase::LightAttack);
			}

			if (PlayerController->HeavyAttackAction)
			{
				EnhancedInput->BindAction(PlayerController->HeavyAttackAction, ETriggerEvent::Started, this,
				                          &AVGCharacterBase::HeavyAttack);
			}
		}
	}
}

void AVGCharacterBase::Move(const FInputActionValue& Value)
{
	if (GetController() != nullptr)
	{
		const FVector2D MovementVector = Value.Get<FVector2D>();

		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);
	}
}

void AVGCharacterBase::StartJump(const FInputActionValue& Value)
{
	Jump();
}

void AVGCharacterBase::StopJump(const FInputActionValue& Value)
{
	StopJumping();
}

void AVGCharacterBase::Look(const FInputActionValue& Value)
{
	if (GetController() != nullptr)
	{
		const FVector2D LookAxisVector = Value.Get<FVector2D>();
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AVGCharacterBase::StartSprint(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	ServerRPCSetSprinting(true);
}

void AVGCharacterBase::StopSprint(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	ServerRPCSetSprinting(false);
}

void AVGCharacterBase::CameraZoom(const FInputActionValue& Value)
{
}

void AVGCharacterBase::LightAttack(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->TryLightAttack();
	}
}

void AVGCharacterBase::HeavyAttack(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->TryHeavyAttack();
	}
}

void AVGCharacterBase::ServerRPCSetSprinting_Implementation(bool bIsSprinting)
{
	// TODO: GameplayTag 체크 필요
	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		// TODO: Gameplay Tag 추가 (e.g. State.Movement.Sprinting)
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
		// TODO: Gameplay Tag 제거
	}
}
