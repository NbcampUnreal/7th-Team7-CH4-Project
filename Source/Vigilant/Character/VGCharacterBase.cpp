#include "Character/VGCharacterBase.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/Component/VGHiddenPocketComponent.h"
#include "Common/VGGameplayTags.h"
#include "Component/VGCombatComponent.h"
#include "Component/VGLockOnComponent.h"
#include "Component/VGStatComponent.h"
#include "Core/Interface/VGGameModeInterface.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/VGUIManagerSubsystem.h"
#include "UI/VGHUDWidget.h"
#include "Core/Interface/VGGameModeInterface.h"
#include "Core/Interface/VGPlayerInfoInterface.h"
#include "Data/VGShieldDataAsset.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerState.h"


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
	  CameraZoomAction(nullptr),
      HiddenPocketAction(nullptr)
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
	CameraBoom->bEnableCameraRotationLag = false;

	// create the orbiting camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// create the combat component
	CombatComponent = CreateDefaultSubobject<UVGCombatComponent>(TEXT("CombatComponent"));

	// create the stat component
	StatComponent = CreateDefaultSubobject<UVGStatComponent>(TEXT("StatComponent"));
	
	LockOnComponent = CreateDefaultSubobject<UVGLockOnComponent>(TEXT("VGLockOnComponent"));
	HiddenPocketComponent = CreateDefaultSubobject<UVGHiddenPocketComponent>(TEXT("HiddenPocketComponent"));
}

void AVGCharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVGCharacterBase, CharacterTags);
}

void AVGCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	if (StatComponent)
	{
		StatComponent->OnStaminaChanged.AddDynamic(this, &AVGCharacterBase::HandleSprintStamina);
	}
	if (LockOnComponent)
	{
		LockOnComponent->OnLockOnTargetChanged.AddUniqueDynamic(this, &AVGCharacterBase::HandleLockOnTargetChanged);
	}
	
	if (StatComponent)
	{
		StatComponent->OnDead.AddDynamic(this, &AVGCharacterBase::HandleDeath);
	}
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
//빙의 후 서버만 실행하는 생명주기 함수
void AVGCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ApplyPlayerMesh();
}

void AVGCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	ApplyPlayerMesh();
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
		
		if (LockOnAction)
		{
			EnhancedInput->BindAction(LockOnAction, ETriggerEvent::Started, this, 
				&AVGCharacterBase::LockOn);
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
		
		if (HiddenPocketAction)
		{
			EnhancedInput->BindAction(HiddenPocketAction, ETriggerEvent::Started, this, &AVGCharacterBase::HiddenPocketToggle);
		}
	}
}


void AVGCharacterBase::ApplyPlayerMesh()
{
	// 데이터 에셋이 안 비어있는지 확인
	if (!CharacterDataAsset)
	{
		return;
	}

	// 내 캐릭터의 PlayerState를 가져와서 번호표(EntryIndex) 확인
	if (IVGPlayerInfoInterface* VGPlayerInfo = Cast<IVGPlayerInfoInterface>(GetPlayerState()))
	{
		int32 PlayerMeshIndex= VGPlayerInfo->GetRandomMeshNumber();
		// 배열은 0번부터 시작하므로 (EntryIndex - 1) 처리
		

		// 해당 인덱스가 배열 범위 내에 안전하게 존재하는지 검사
		if (CharacterDataAsset->PlayerMeshes.IsValidIndex(PlayerMeshIndex))
		{
			// 메쉬 강제 교체
			USkeletalMesh* SelectedMesh = CharacterDataAsset->PlayerMeshes[PlayerMeshIndex];
			GetMesh()->SetSkeletalMesh(SelectedMesh);
		}
	}
}

void AVGCharacterBase::Move(const FInputActionValue& Value)
{
	if (CharacterTags.HasTag(VigilantCharacter::Attacking) || CharacterTags.HasTag(VigilantCharacter::Dodge) ||
		CharacterTags.HasTag(VigilantCharacter::Stunned) || CharacterTags.HasTag(VigilantCharacter::Guard))
	{
		return;
	}

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
	if (CharacterTags.HasTag(VigilantCharacter::LockOn))
	{
		return;
	}
	if (GetController() != nullptr)
	{
		const FVector2D LookAxisVector = Value.Get<FVector2D>();
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

#pragma region 락온 관련 함수 구현
void AVGCharacterBase::LockOn(const FInputActionValue& Value)
{
	
	UE_LOG(LogTemp, Warning, TEXT("[%s] E키 입력 감지됨!"), *GetName());
	
	if (LockOnComponent)
	{
		LockOnComponent->LockOnPerform();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("클라이언트 인스턴스(%s)의 LockOnComponent가 nullptr입니다!"), *GetName());
	}
}
void AVGCharacterBase::Server_SetLockOnTag_Implementation(bool bIsLockedOn)
{
	if (bIsLockedOn)
	{
		CharacterTags.AddTag(VigilantCharacter::LockOn);
	}
	else
	{
		CharacterTags.RemoveTag(VigilantCharacter::LockOn);
	}
}
void AVGCharacterBase::HandleLockOnTargetChanged(AActor* NewTarget)
{
	if (NewTarget)
	{
		// 락온 성공 시
		CharacterTags.AddTag(VigilantCharacter::LockOn);
		Server_SetLockOnTag(true);
		SetCharacterRotationState(true); // 락온용 회전 상태 
	}
	else
	{
		// 락온 해제 시
		CharacterTags.RemoveTag(VigilantCharacter::LockOn);
		Server_SetLockOnTag(false);
		SetCharacterRotationState(false); // 일반 이동용 회전 상태
	}
}
#pragma endregion

#pragma region 스프린트 관련 함수 구현
void AVGCharacterBase::StartSprint(const FInputActionValue& Value)
{
	//게임플레이 태그 검사, 스태미나 검사
	if (CharacterTags.HasTag(VigilantCharacter::Sprint) || CharacterTags.HasTag(VigilantCharacter::Stunned) || CharacterTags.HasTag(VigilantCharacter::Attacking))
	{
		return;
	}
	if (StatComponent->GetCurrentStamina() < MinStaminaToSprint)
	{
		return;
	}
	
	// 잠겨있다면 잠시 풀기
	if (CharacterTags.HasTag(VigilantCharacter::LockOn))
	{
		//SetCharacterRotationState(false);
	}
	
	bWantsToSprint = true;

	PerformStartSprint();
	Server_StartSprint();
}

void AVGCharacterBase::StopSprint(const FInputActionValue& Value)
{
	//회전잠금해제
	if (CharacterTags.HasTag(VigilantCharacter::LockOn))
	{
		UE_LOG(LogTemp, Warning, TEXT("잠금"));
		//SetCharacterRotationState(true);
	}
	
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
#pragma endregion

void AVGCharacterBase::CameraZoom(const FInputActionValue& Value)
{
}

void AVGCharacterBase::LightAttack(const FInputActionValue& Value)
{
	// 페이즈 체크 후 실행
	if (!IsCombatActionAllowed()) return;
	
	if (CombatComponent)
	{
		CombatComponent->TryLightAttack();
	}
}

void AVGCharacterBase::HeavyAttack(const FInputActionValue& Value)
{
	// 페이즈 체크 후 실행
	if (!IsCombatActionAllowed()) return;
	
	if (CombatComponent)
	{
		CombatComponent->TryHeavyAttack();
	}
}

void AVGCharacterBase::HiddenPocketToggle(const FInputActionValue& Value)
{
	if (HiddenPocketComponent)
	{
		HiddenPocketComponent->TogglePocket();
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
	// 현재 페이즈가 데미지를 받는게 허용되는 페이즈인지 확인
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (GameMode && GameMode->Implements<UVGGameModeInterface>())
	{
		if (!IVGGameModeInterface::Execute_CanPlayerTakeDamage(GameMode, DamageCauser, this))
		{
			return 0.0f;
		}
	}
	
	// 1. 무적 상태 확인
	if (CharacterTags.HasTag(VigilantCharacter::Invincible))
	{
		return 0.0f;
	}

	// 공격자->방어자 방향 계산
	FVector PushDirection = FVector::ZeroVector;
	if (DamageCauser)
	{
		PushDirection = GetActorLocation() - DamageCauser->GetActorLocation();
		PushDirection.Z = 0.0f;
		PushDirection.Normalize();
	}

	// --- 2. 미션 페이즈: 데미지 적용 X, 밀치기 O ---
	if (AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		if (IGameplayTagAssetInterface* GameStateTag = Cast<IGameplayTagAssetInterface>(GameState))
		{
			if (GameStateTag->HasMatchingGameplayTag(VigilantPhaseTags::PhaseMission) || GameStateTag->
				HasMatchingGameplayTag(VigilantPhaseTags::PhaseLobby))
			{
				ApplyStagger(PushDirection, 800.0f);
				return 0.0f;
			}
		}
	}
	
	// 공격이 정면에서 들어오는지 확인
	bool bIsFrontAttack = false;
	if (DamageCauser)
	{
		FVector ToAttacker = -PushDirection; // 공격자를 가리키는 방향
		float DotResult = FVector::DotProduct(GetActorForwardVector(), ToAttacker);
		bIsFrontAttack = DotResult > 0.5f;
	}
	
	if (bIsFrontAttack)
	{
		// --- 3. 패리 확인 ---
		if (CharacterTags.HasTag(VigilantCharacter::PerfectGuard))
		{
			if (AVGCharacterBase* Attacker = Cast<AVGCharacterBase>(DamageCauser))
			{
				FVector ReversePushDirection = -PushDirection;
				Attacker->ApplyStagger(ReversePushDirection, 600.0f);
			}
			// TODO: SFX, VFX 추가
			return 0.0f;
		}

		// --- 4. 일반 가드 확인 ---
		if (CharacterTags.HasTag(VigilantCharacter::Guard))
		{
			if (!CombatComponent)
			{
				return 0.0f;
			}
			
			if (UVGShieldDataAsset* ShieldData = CombatComponent->GetCurrentShieldData())
			{
				DamageAmount *= ShieldData->DamageMitigation;
			}
		}
	}
	
	// --- 5. 피해 적용 ---
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (StatComponent && ActualDamage > 0.f)
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

bool AVGCharacterBase::IsCombatActionAllowed() const
{
	if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
	{
		if (GameMode->Implements<UVGGameModeInterface>())
		{
			return IVGGameModeInterface::Execute_CanPlayerAttack(GameMode, const_cast<AVGCharacterBase*>(this));
		}
	}
	
	return true;
}

bool AVGCharacterBase::IsInteractionAllowed(AActor* Target) const
{
	if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
	{
		if (GameMode->Implements<UVGGameModeInterface>())
		{
			return IVGGameModeInterface::Execute_CanPlayerInteract(GameMode, const_cast<AVGCharacterBase*>(this), Target);
		}
	}
	
	return true;
}

void AVGCharacterBase::Client_ForceRotation_Implementation(FRotator NewRotation)
{
	// z값 제외 전부 무시
	FRotator SafeRotation = FRotator(0.0f, NewRotation.Yaw, 0.0f);

	// 물리 충돌 무시
	SetActorRotation(SafeRotation, ETeleportType::TeleportPhysics);

	// 카메라 회전
	if (AController* PlayerController = GetController())
	{
		PlayerController->SetControlRotation(SafeRotation);
	}
	// 관성 삭제
	if (UCharacterMovementComponent* CharacterMovementComponent = Cast<UCharacterMovementComponent>(GetMovementComponent()))
	{
		CharacterMovementComponent->Velocity = FVector::ZeroVector;
	}
	
	// 몸통이 무조간 카메라 방향보도록 고정 후 0.1초 후에 풀기
	bUseControllerRotationYaw = true;
	FTimerHandle SyncTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		SyncTimerHandle, 
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (this)
			{
				bUseControllerRotationYaw = false;
			}
		}), 
		0.1f, 
		false
	);
}

bool AVGCharacterBase::CanInteract_Implementation(AActor* Interactor) const
{
	return true;
}

void AVGCharacterBase::OnInteract_Implementation(AActor* Interactor, const FTransform& InteractTransform)
{
	if (!HasAuthority()) return;

	if (AVGCharacterBase* VGChallenger = Cast<AVGCharacterBase>(Interactor))
	{
		VGChallenger->NotifyPlayerInteraction(this);
	}
}

void AVGCharacterBase::HandleDeath(AController* Killer)
{
	if (HasAuthority())
	{
		AVGCharacterBase* KillerCharacter = nullptr;
		if (Killer && Killer->GetPawn())
		{
			KillerCharacter = Cast<AVGCharacterBase>(Killer->GetPawn());
		}

		// 게임모드 인터페이스를 통해 보고
		AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
		if (GameMode && GameMode->Implements<UVGGameModeInterface>())
		{
			IVGGameModeInterface::Execute_NotifyPlayerDeath(GameMode, KillerCharacter, this);
		}
	}
}

void AVGCharacterBase::ApplyStagger(FVector PushDirection, float KnockbackForce)
{
	if (!HasAuthority() || CharacterTags.HasTag(VigilantCharacter::Invincible))
	{
		return;
	}

	LaunchCharacter(PushDirection * KnockbackForce, true, true);
	Multicast_PlayStaggerVisual();
}

void AVGCharacterBase::Multicast_PlayStaggerVisual_Implementation()
{
	StopAnimMontage();
	if (CombatComponent)
	{
		CombatComponent->StopAttackExecution();
	}

	if (StaggerMontage)
	{
		PlayAnimMontage(StaggerMontage);
	}
}

void AVGCharacterBase::ServerRPCSetSprinting_Implementation(bool bIsSprinting)
{
}

void AVGCharacterBase::SetCharacterRotationState(bool bIsLockedOn)
{
	// 캐릭터의 bOrientRotationToMovement 등을 상황에 맞게 전환
	// 누가 언제 이 함수를 호출하는지 추적
	UE_LOG(LogTemp, Warning, TEXT("[%s] SetCharacterRotationState 호출됨! 변경된 값: %s"), 
		*GetName(), bIsLockedOn ? TEXT("TRUE") : TEXT("FALSE"));
	
	GetCharacterMovement()->bOrientRotationToMovement = !bIsLockedOn;
	bUseControllerRotationYaw = bIsLockedOn;
}