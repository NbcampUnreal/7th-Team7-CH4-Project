#include "Character/VGCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/GameModeBase.h"
#include "DrawDebugHelpers.h"
#include "Common/VGGameplayTags.h"
#include "Component/VGCombatComponent.h"
#include "Component/VGStatComponent.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/VGUIManagerSubsystem.h"
#include "UI/VGHUDWidget.h"
#include "Core/Interface/VGGameModeInterface.h"


#pragma region Interfaces GameplayTag
void AVGCharacterBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = CharacterTags;
}

void AVGCharacterBase::AddGameplayTag(FGameplayTag TagToAdd)
{
	CharacterTags.AddTag(TagToAdd);
}

void AVGCharacterBase::RemoveGameplayTag(FGameplayTag TagToRemove)
{
	CharacterTags.RemoveTag(TagToRemove);
}
#pragma endregion

AVGCharacterBase::AVGCharacterBase()
	: JumpAction(nullptr),
	  MoveAction(nullptr),
	  LookAction(nullptr),
	  SprintAction(nullptr),
	  CameraZoomAction(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

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

void AVGCharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVGCharacterBase, CharacterTags);
}

void AVGCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	StatComponent->OnStaminaChanged.AddDynamic(this, &AVGCharacterBase::HandleSprintStamina);
}

//빙의 후 클라이언트만 실행하는 생명주기 함수
void AVGCharacterBase::PawnClientRestart()
{
	Super::PawnClientRestart();
	//컨트롤러->로컬플레이어->로컬플레이어서브시스템(UI매니저) -> HUDInstance 로 연결
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UVGUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
			{
				//스탯컴포넌트와 UI매니저 바인딩
				StatComponent->OnStaminaChanged.AddDynamic(UIManager, &UVGUIManagerSubsystem::OnStaminaUpdate);
				StatComponent->OnHPChanged.AddDynamic(UIManager, &UVGUIManagerSubsystem::OnHealthUpdate);

				//초기값 설정 요청
				UIManager->OnStaminaUpdate(StatComponent->GetCurrentStamina(), StatComponent->GetMaxStamina());
				UIManager->OnHealthUpdate(StatComponent->GetCurrentHP(), StatComponent->GetMaxHP());
			}
		}
	}
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

		if (CombatComponent)
		{
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
	//게임플레이 태그 검사, 스태미나 검사
	if (CharacterTags.HasTag(VigilantCharacter::Sprint))
	{
		return;
	}
	if (StatComponent->GetCurrentStamina() < MinStaminaToSprint)
	{
		return;
	}

	bWantsToSprint = true;

	PerformStartSprint();
	Server_StartSprint();
}

void AVGCharacterBase::StopSprint(const FInputActionValue& Value)
{
	bWantsToSprint = false;
	if (CharacterTags.HasTag(VigilantCharacter::Sprint))
	{
		PerformStopSprint();
		Server_StopSprint();
	}
}

void AVGCharacterBase::PerformStartSprint()
{
	CharacterTags.AddTag(VigilantCharacter::Sprint);

	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AVGCharacterBase::PerformStopSprint()
{
	CharacterTags.RemoveTag(VigilantCharacter::Sprint);
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void AVGCharacterBase::Server_StartSprint_Implementation()
{
	if (StatComponent)
	{
		StatComponent->StartContinuousConsumeStamina(SprintStaminaCostPerSecond);
	}
	PerformStartSprint();
}

void AVGCharacterBase::Server_StopSprint_Implementation()
{
	if (StatComponent)
	{
		StatComponent->StopContinuousConsumeStamina();
	}
	PerformStopSprint();
}

void AVGCharacterBase::HandleSprintStamina(float CurrentStamina, float Max)
{
	if (CurrentStamina <= 0.f && CharacterTags.HasTag(VigilantCharacter::Sprint))
	{
		if (IsLocallyControlled())
		{
			PerformStopSprint();
			Server_StopSprint();
		}
	}
	if (bWantsToSprint && CurrentStamina > MinStaminaToSprint && !CharacterTags.HasTag(VigilantCharacter::Sprint))
	{
		PerformStartSprint();
		Server_StartSprint();
	}
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


void AVGCharacterBase::OnRep_CharacterTags()
{
	//UE_LOG(LogTemp, Log, TEXT("클라이언트: 캐릭터 태그가 서버로부터 갱신되었습니다!"));
	//아직은 쓸데가 없음..
}


float AVGCharacterBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                   class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);


	if (CharacterTags.HasTag(VigilantCharacter::Invincible))
	{
		//구르기 무적
		ActualDamage = 0;
		return ActualDamage;
	}

	if (StatComponent)
	{
		StatComponent->ApplyDamageToStat(ActualDamage, EventInstigator);
	}

	return ActualDamage;
}

void AVGCharacterBase::NotifyPlayerInteraction(class AVGCharacterBase* TargetPlayer)
{
	// 서버에서만 실행
	if (!HasAuthority()) return; 

	AGameModeBase* CurrentGameMode = GetWorld()->GetAuthGameMode();
    
	// 게임모드 인터페이스를 통해 막고라 호출 함수 호출
	if (CurrentGameMode && CurrentGameMode->Implements<UVGGameModeInterface>())
	{
		IVGGameModeInterface::Execute_RequestDuelPhase(CurrentGameMode, this, TargetPlayer);
	}
}

void AVGCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
