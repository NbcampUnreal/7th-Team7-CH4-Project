#include "Character/VGCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Common/VGGameplayTags.h"
#include "Component/VGCombatComponent.h"
#include "Component/VGStatComponent.h"

AVGCharacterBase::AVGCharacterBase()
: JumpAction(nullptr),
  MoveAction(nullptr),
  LookAction(nullptr),
  SprintAction(nullptr),
  CameraZoomAction(nullptr)
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
	
	// create the stat component
	StatComponent = CreateDefaultSubobject<UVGStatComponent>(TEXT("StatComponent"));
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
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this,
			                          &AVGCharacterBase::Move);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this,
			                          &AVGCharacterBase::Look);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this,
			                          &AVGCharacterBase::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this,
			                          &AVGCharacterBase::StopJump);
		}
		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this,
			                          &AVGCharacterBase::StartSprint);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this,
			                          &AVGCharacterBase::StopSprint);
		}
		
		
		if (CombatComponent->LightAttackAction)
		{
			EnhancedInput->BindAction(CombatComponent->LightAttackAction, ETriggerEvent::Started, this,
			                          &AVGCharacterBase::LightAttack);
		}

		if (CombatComponent->HeavyAttackAction)
		{
			EnhancedInput->BindAction(CombatComponent->HeavyAttackAction, ETriggerEvent::Started, this,
			                          &AVGCharacterBase::HeavyAttack);
		}
	}
}

void AVGCharacterBase::PawnClientRestart()
{
	Super::PawnClientRestart();
	//카메라 각도 제한
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->ViewPitchMax = 60.f;
			PlayerController->PlayerCameraManager->ViewPitchMin = -45.f;
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
