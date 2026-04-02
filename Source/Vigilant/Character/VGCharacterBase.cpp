#include "Character/VGCharacterBase.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Core/VGPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Component/VGEquipmentComponent.h"
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
	
	EquipmentComponent = CreateDefaultSubobject<UVGEquipmentComponent>(TEXT("EquipmentComponent"));
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
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this,
				                          &AVGCharacterBase::Move);

			if (PlayerController->LookAction)
				EnhancedInput->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this,
				                          &AVGCharacterBase::Look);

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
			
			if (InteractAction)
			{
				// F키를 눌렀을 때 Interact 함수 실행
				EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AVGCharacterBase::Interact);
			}

			if (DropAction)
			{
				// G키를 눌렀을 때 DropItem 함수 실행
				EnhancedInput->BindAction(DropAction, ETriggerEvent::Started, this, &AVGCharacterBase::DropItem);
			}
			
			if (SlotSelectAction)
			{
				// 1, 2키를 눌렀을 때 SelectSlot 함수 실행
				EnhancedInput->BindAction(SlotSelectAction, ETriggerEvent::Started, this, &AVGCharacterBase::SelectSlot);
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

void AVGCharacterBase::Interact()
{
	// 로컬 플레이어 화면에서만 레이저를 쏘도록 검사
	if (!IsLocallyControlled()) return;
	if (FollowCamera == nullptr || EquipmentComponent == nullptr) return;

	// 레이저 시작점 (카메라 위치)과 끝점 (카메라가 보는 방향으로 1500cm 앞) 계산
	FVector StartLocation = FollowCamera->GetComponentLocation();
	FVector ForwardVector = FollowCamera->GetForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * 1500.0f); // 💡 숫자를 조절해서 줍는 거리 조절 가능

	// 레이저 충돌 결과를 담을 그릇
	FHitResult HitResult;
    
	// 나 자신은 레이저에 맞지 않게 예외 처리
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	// 디버그 코드
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(30.0f); // 숫자를 키우면 줍기 판정이 더 후해짐

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult, 
		StartLocation, 
		EndLocation, 
		FQuat::Identity, 
		ECC_Visibility, 
		SphereShape, 
		CollisionParams
	);
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f);
	
	if (bHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.Location, 30.0f, 16, FColor::Green, false, 2.0f);
	}
	else
	{
		DrawDebugSphere(GetWorld(), EndLocation, 30.0f, 16, FColor::Red, false, 2.0f);
	}
	// 디버그 코드 끝
	
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
        
		// 레이저에 맞은 액터가 '장착 가능한 아이템(AVGEquippableActor)'인지 캐스팅해서 확인
		if (AVGEquippableActor* EquippableItem = Cast<AVGEquippableActor>(HitActor))
		{
			// 아이템 이름으로 임시 타입 구분 (아이템 로직 완성 전까지 사용)
			EVGEquipmentType TypeToEquip = EVGEquipmentType::Weapon; // 기본은 무기
            
			// 블루프린트 이름에 "Mission"이 들어가 있다면 미션 아이템으로 취급
			if (EquippableItem->GetName().Contains("Mission"))
			{
				TypeToEquip = EVGEquipmentType::MissionItem;
			}
			// 장착 요청
			EquipmentComponent->Server_EquipItem(EquippableItem, TypeToEquip);

			// 장착 타입에 맞춰서 활성화 슬롯 자동 동기화
			if (TypeToEquip == EVGEquipmentType::Weapon)
			{
				ActiveEquipmentSlot = EVGEquipmentSlot::RightHand;
				UE_LOG(LogTemp, Warning, TEXT("무기 장착 활성화 슬롯이 [오른손]으로 강제 전환"));
				
				OnEquipmentSlotChanged.Broadcast(ActiveEquipmentSlot);
			}
			else if (TypeToEquip == EVGEquipmentType::Shield)
			{
				ActiveEquipmentSlot = EVGEquipmentSlot::LeftHand;
				UE_LOG(LogTemp, Warning, TEXT("방패 장착 활성화 슬롯이 [왼손]으로 강제 전환"));
				
				OnEquipmentSlotChanged.Broadcast(ActiveEquipmentSlot);
			}
			// 미션 아이템은 빈 손에 들어가므로 활성화 슬롯을 강제로 바꾸지 않음
            
			UE_LOG(LogTemp, Log, TEXT("레이저 적중 아이템 줍기 요청: %s"), *EquippableItem->GetName());
		}
	}
}

void AVGCharacterBase::DropItem()
{
	if (EquipmentComponent)
	{
		// 이제 무조건 오른손을 버리는 게 아니라, '현재 활성화된 슬롯'을 버림
		EquipmentComponent->Server_DropItem(ActiveEquipmentSlot);
		UE_LOG(LogTemp, Log, TEXT("현재 활성화된 슬롯의 아이템 버리기"));
	}
}

void AVGCharacterBase::SelectSlot(const FInputActionValue& Value)
{
	float SlotNumber = Value.Get<float>();

	// 1.0 이면 왼손, 2.0 이면 오른손 활성화
	if (FMath::IsNearlyEqual(SlotNumber, 1.0f))
	{
		ActiveEquipmentSlot = EVGEquipmentSlot::LeftHand;
		UE_LOG(LogTemp, Warning, TEXT("왼손 슬롯 활성화"));
		
		OnEquipmentSlotChanged.Broadcast(ActiveEquipmentSlot);
	}
	else if (FMath::IsNearlyEqual(SlotNumber, 2.0f))
	{
		ActiveEquipmentSlot = EVGEquipmentSlot::RightHand;
		UE_LOG(LogTemp, Warning, TEXT("오른손 슬롯 활성화"));
		
		OnEquipmentSlotChanged.Broadcast(ActiveEquipmentSlot);
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
