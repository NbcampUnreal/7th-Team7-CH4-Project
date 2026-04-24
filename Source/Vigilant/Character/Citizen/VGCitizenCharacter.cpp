// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Citizen/VGCitizenCharacter.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "Character/Component/VGCombatComponent.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Character/Component/VGHiddenPocketComponent.h"
#include "Character/Component/VGStatComponent.h"
#include "Common/VGGameplayTags.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Core/Interface/VGUIControllerInterface.h"
#include "Data/VGShieldDataAsset.h"
#include "Data/VGWeaponDataAsset.h"
#include "Data/VGCharacterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/VGUIManagerSubsystem.h"
#include "Character/Boss/DamageType/VGDamageType_Slow.h" 
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "Materials/MaterialInstanceDynamic.h"


class UVGDamageType_Slow;

AVGCitizenCharacter::AVGCitizenCharacter()
{
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	//속도 조정
	NormalSpeed = 600.f;
	SprintSpeed = 900.f;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	
	GetCharacterMovement()->BrakingDecelerationWalking = 1024.f;
	OriginalFriction = GetCharacterMovement()->GroundFriction;
	ModifyFriction = 0.f;
	// 장비 컴포넌트 생성
	EquipmentComponent = CreateDefaultSubobject<UVGEquipmentComponent>(TEXT("EquipmentComponent"));
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
	SceneCaptureComponent->SetupAttachment(RootComponent);
	
	EmoteWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EmoteWidgetComponent"));
	EmoteWidgetComponent->SetupAttachment(RootComponent);
	EmoteWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen); // 항상 카메라를 바라보는 스크린 모드
	EmoteWidgetComponent->SetDrawSize(FVector2D(100.f, 100.f)); // UI 크기 지정
	EmoteWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f)); // 머리 위 Z축 오프셋
	EmoteWidgetComponent->SetVisibility(false); // 기본 상태는 숨김
}

void AVGCitizenCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (EquipmentComponent)
	{
		EquipmentComponent->OnItemEquipped.AddDynamic(this, &AVGCitizenCharacter::HandleItemEquipped);
		EquipmentComponent->OnItemDropped.AddDynamic(this, &AVGCitizenCharacter::HandleItemDropped);

		//컨트롤러->로컬플레이어->로컬플레이어서브시스템(UI매니저) -> HUDInstance 로 연결 바인딩
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
			{
				if (UVGUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
				{
					EquipmentComponent->OnEquipmentSlotChanged.AddDynamic(
						UIManager, &UVGUIManagerSubsystem::EquipSlotChanged);
				}
				
				
					EquipmentComponent->OnInteractTargetFound.AddDynamic(
						this, &AVGCitizenCharacter::HandleInteractFound);
				
				//
				if (UVGHiddenPocketComponent* PocketComp = GetComponentByClass<UVGHiddenPocketComponent>())
				{
					PocketComp->OnPocketItemStashed.AddDynamic(this, &AVGCitizenCharacter::HandlePocketItemStashed);
					PocketComp->OnPocketItemDropped.AddDynamic(this, &AVGCitizenCharacter::HandlePocketItemDropped);
				}
				
			}
			
						
		}
	}
	
	if (CombatComponent)
	{
		CombatComponent->OnGuardStateChanged.AddDynamic(this, &AVGCitizenCharacter::ApplyGuardStaminaCost);
	}

	if (StatComponent)
	{
		StatComponent->OnStaminaChanged.AddDynamic(this, &AVGCitizenCharacter::CheckGuardBreakOnStaminaChanged);
	}
}

void AVGCitizenCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


void AVGCitizenCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 자식 클래스만의 전용 입력 바인딩 추가
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InteractAction)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AVGCitizenCharacter::Interact);
		}
		if (DropAction)
		{
			EnhancedInput->BindAction(DropAction, ETriggerEvent::Started, this, &AVGCitizenCharacter::DropItem);
		}
		if (SlotSelectAction)
		{
			EnhancedInput->BindAction(SlotSelectAction, ETriggerEvent::Started, this, &AVGCitizenCharacter::SelectSlot);
		}
		if (DodgeAction)
		{
			EnhancedInput->BindAction(DodgeAction, ETriggerEvent::Started, this, &AVGCitizenCharacter::Dodge);
		}
		if (BlockAction)
		{
			EnhancedInput->BindAction(BlockAction, ETriggerEvent::Started, this, &AVGCitizenCharacter::StartBlock);
			EnhancedInput->BindAction(BlockAction, ETriggerEvent::Completed, this, &AVGCitizenCharacter::StopBlock);
		}
		if (HeyAction)
		{
			EnhancedInput->BindAction(HeyAction, ETriggerEvent::Started, this, &AVGCitizenCharacter::Hey);
		}
	}
}

void AVGCitizenCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	// 
	if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		CharacterTags.AddTag(VigilantCharacter::Falling);
	}
	// PreMovementMode 이전 상태를 뜻함
	else if (PrevMovementMode == MOVE_Falling)
	{
		// 공중 상태 태그 제거
		CharacterTags.RemoveTag(VigilantCharacter::Falling);
	}
}

void AVGCitizenCharacter::Interact()
{
	if (!CanInteract())
	{
		return;
	}
	
	AActor* Target = EquipmentComponent->GetCurrentInteractableTarget();
	if (!IsInteractionAllowed(Target))
	{
		return; 
	}

	if (EquipmentComponent)
	{
		EquipmentComponent->Interact();
	}
}

void AVGCitizenCharacter::DropItem()
{
	if (!CanInteract())
	{
		return;
	}
	
	if (EquipmentComponent)
	{
		EquipmentComponent->DropItem();
	}
}

void AVGCitizenCharacter::SelectSlot(const FInputActionValue& Value)
{
	float SlotNumber = Value.Get<float>();
	if (EquipmentComponent)
	{
		EquipmentComponent->SelectSlot(SlotNumber);
	}
}

void AVGCitizenCharacter::HandleInteractFound(const FString& InfoText, const FVector& TargetLocation, bool bShow)
{
	if (IVGUIControllerInterface* UIControllerInterface= Cast<IVGUIControllerInterface>(GetController()))
	{
		UIControllerInterface->ShowInteractUI(InfoText, TargetLocation, bShow);
	}
}

void AVGCitizenCharacter::Move(const FInputActionValue& Value)
{
	Super::Move(Value);
}


void AVGCitizenCharacter::StartBlock(const FInputActionValue& Value)
{
	if (!CanBlock())
	{
		return;
	}
	
	if (!CombatComponent || !StatComponent)
	{
		return;
	}
	
	UVGShieldDataAsset* ShieldData = CombatComponent->GetCurrentShieldData();
	if (!ShieldData)
	{
		return;
	}
	
	if (StatComponent->GetCurrentStamina() < ShieldData->BlockActivationStaminaCost)
	{
		// TODO: SFX 적용
		return;
	}
	
	CombatComponent->TryStartBlock();
}

void AVGCitizenCharacter::StopBlock(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->TryStopBlock();
	}
}

#pragma region State Check
bool AVGCitizenCharacter::CanDodge() const
{
	// 회피는 공격을 중단할 수 있음
	return !CharacterTags.HasTag(VigilantCharacter::Stunned) &&
		!CharacterTags.HasTag(VigilantCharacter::Dodge) &&
		!CharacterTags.HasTag(VigilantCharacter::Falling);
}

bool AVGCitizenCharacter::CanInteract() const
{
	return !CharacterTags.HasTag(VigilantCharacter::Attacking) &&
		!CharacterTags.HasTag(VigilantCharacter::Dodge) &&
		!CharacterTags.HasTag(VigilantCharacter::Stunned) &&
		!CharacterTags.HasTag(VigilantCharacter::Guard);
}

bool AVGCitizenCharacter::CanBlock() const
{
	return !CharacterTags.HasTag(VigilantCharacter::Dodge) &&
		!CharacterTags.HasTag(VigilantCharacter::Stunned);
}

#pragma endregion

#pragma region 구르기 로직 및 RPC
void AVGCitizenCharacter::Dodge()
{
	if (!CanDodge())
	{
		return;
	}
	if (StatComponent->GetCurrentStamina() < 2.f) // 최소 사용 가능 스태미너, 일단 하드코딩
	{
		return;
	}
	// 잠겨있다면 회전 잠시 풀기
	if (CharacterTags.HasTag(VigilantCharacter::LockOn))
	{
		SetCharacterRotationState(false);
	}
	
	CharacterTags.AddTag(VigilantCharacter::Dodge);
	
	
	
	//방향 계산
	FVector DodgeDirection = GetCharacterMovement()->GetLastInputVector();
	if (DodgeDirection.IsNearlyZero())
	{
		DodgeDirection = GetActorForwardVector();
	}
	DodgeDirection.Normalize();

	//RPC 호출 부분
	if (!HasAuthority()) // 서버가 아니면
	{
		PerformDodgeAction(DodgeDirection); // 실제 로직 실행 - 클라가 구른다.
		Server_Dodge(DodgeDirection); //서버에서 계산할 함수 실행
	}
	else //서버면 - 리슨서버 대비용 구문
	{
		PerformDodgeAction(DodgeDirection); //실제 로직 실행 - 마찰력과 위치를 중점
		Multicast_Dodge(); // 멀티캐스트 실행
	}
}

void AVGCitizenCharacter::PerformDodgeAction(const FVector& Direction)
{
	CharacterTags.AddTag(VigilantCharacter::Dodge);
	
	if (StatComponent)
	{
		StatComponent->ConsumeStamina(20.f); //일단하드코딩
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(DodgeAnimation);

		FOnMontageBlendingOutStarted BlendingOutStarted;
		BlendingOutStarted.BindUObject(this, &AVGCitizenCharacter::OnMontageCompleted);
		AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutStarted, DodgeAnimation);

		//루트모션이 아닌 직접 날리자.. 멀티플레이상황에서는 루트모션이 버벅거림


		FVector DodgeVelocity = Direction * DodgeForce;
		DodgeVelocity.Z = DodgeZForce;
		LaunchCharacter(DodgeVelocity, true, true);

		FRotator DodgeRotation = Direction.Rotation();
		DodgeRotation.Pitch = 0.f;
		DodgeRotation.Roll = 0.f;
		SetActorRotation(DodgeRotation);
		//구르기 느낌을 위한 마찰력 조절
		GetCharacterMovement()->GroundFriction = ModifyFriction;
	}
}



void AVGCitizenCharacter::Multicast_Dodge_Implementation()
{
	//다른 클라이언트에서는 애니매이션만 재생
	if (!IsLocallyControlled() && !HasAuthority()) // 서버도 로컬도 아닐때, 즉 이 액터가 Simulated일때
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(DodgeAnimation);
		}
	}
}

void AVGCitizenCharacter::Server_Dodge_Implementation(FVector Direction)
{
	PerformDodgeAction(Direction);
	Multicast_Dodge();
}
#pragma endregion 김형백

void AVGCitizenCharacter::OnMontageCompleted(UAnimMontage* Montage, bool bWasCancelled)
{
	CharacterTags.RemoveTag(VigilantCharacter::Dodge);
	GetCharacterMovement()->GroundFriction = OriginalFriction;
	
	// 잠겨있다면 다시 잠궈주기
	if (CharacterTags.HasTag(VigilantCharacter::LockOn))
	{
		UE_LOG(LogTemp, Warning, TEXT("잠금"));
		SetCharacterRotationState(true);
	}
	
	if (bWasCancelled == true)
	{
		//회피가 불명의 이유로 중단되었을때 로직
	}
	else
	{
		//회피가 잘 끝났을 때 로직
	}
}

void AVGCitizenCharacter::HandleItemEquipped(EVGEquipmentSlot Slot, UVGEquipmentDataAsset* EquipmentData,
                                             UMeshComponent* EquippedMesh)
{
	if (!EquipmentData || !CombatComponent)
	{
		return;
	}

	if (UVGWeaponDataAsset* WeaponData = Cast<UVGWeaponDataAsset>(EquipmentData))
	{
		CombatComponent->SetActiveCombatData(WeaponData, EquippedMesh);
	}
	else if (UVGShieldDataAsset* ShieldData = Cast<UVGShieldDataAsset>(EquipmentData))
	{
		CombatComponent->SetActiveShieldData(ShieldData);
	}
	
	//아이콘정보 전달
	if (IVGUIControllerInterface* UIController = Cast<IVGUIControllerInterface>(GetController()))
	{
		int32 SlotIndex = (Slot == EVGEquipmentSlot::LeftHand) ? 1 : 2;
		if (EquipmentData)
		{
			UIController->UpdateEquipIconUI(SlotIndex, EquipmentData->ItemIcon);
		}
	}
	
	
}

void AVGCitizenCharacter::HandlePocketItemStashed(UVGEquipmentDataAsset* EquipmentData, UMeshComponent* EquippedMesh)
{
	if (EquipmentData && EquipmentData->ItemIcon)
	{
		if (IVGUIControllerInterface* UIController = Cast<IVGUIControllerInterface>(GetController()))
		{
			// 데이터 에셋의 아이콘을 인터페이스로 토스
			UIController->UpdateHiddenPocketIconUI(EquipmentData->ItemIcon);
		}
	}
}

void AVGCitizenCharacter::HandlePocketItemDropped()
{
	if (IVGUIControllerInterface* UIController = Cast<IVGUIControllerInterface>(GetController()))
	{
		// 주머니가 비었으니 UI를 지우라고 지시
		UIController->ClearHiddenPocketIconUI();
	}
}

void AVGCitizenCharacter::HandleItemDropped(EVGEquipmentSlot Slot)
{
	if (!CombatComponent)
	{
		return;
	}

	if (Slot == EVGEquipmentSlot::RightHand || Slot == EVGEquipmentSlot::BothHands)
	{
		CombatComponent->SetActiveCombatData(nullptr, nullptr);
	}
	if (Slot == EVGEquipmentSlot::LeftHand || Slot == EVGEquipmentSlot::BothHands)
	{
		CombatComponent->SetActiveShieldData(nullptr);
	}
	
	
	//아이템 드롭 정보 전달
	if (IVGUIControllerInterface* UIController = Cast<IVGUIControllerInterface>(GetController()))
	{
		if (Slot == EVGEquipmentSlot::BothHands)
		{
			UIController->ClearEquipIconUI(1);
			UIController->ClearEquipIconUI(2);
		}
		else
		{
			int32 SlotIndex = (Slot == EVGEquipmentSlot::LeftHand) ? 1 : 2;
			UIController->ClearEquipIconUI(SlotIndex);
		}
	}
	
}

void AVGCitizenCharacter::CheckGuardBreakOnStaminaChanged(float CurrentStamina, float MaxStamina)
{
	if (CurrentStamina <= 0.f && CharacterTags.HasTag(VigilantCharacter::Guard))
	{
		if (IsLocallyControlled())
		{
			if (CombatComponent)
			{
				CombatComponent->TryStopBlock();
			}
		}
		
		if (HasAuthority())
		{
			StatComponent->StopContinuousConsumeStamina();
			ApplyStagger(FVector::ZeroVector, 0.0f);
		}
	}

}

void AVGCitizenCharacter::ApplyGuardStaminaCost(bool bIsGuarding)
{
	if (!HasAuthority() || !StatComponent || !CombatComponent)
	{
		return;
	}
	
	UVGShieldDataAsset* ShieldData = CombatComponent->GetCurrentShieldData();
	if (!ShieldData)
	{
		return;
	}
	
	if (bIsGuarding)
	{
		StatComponent->StartContinuousConsumeStamina(ShieldData->BlockStaminaDrainPerSecond);
		StatComponent->ConsumeStamina(ShieldData->BlockActivationStaminaCost);
	}
	else
	{
		StatComponent->StopContinuousConsumeStamina();
	}
}

float AVGCitizenCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageEvent.DamageTypeClass)
	{
		if (const UVGDamageType_Slow* SlowDamageType = Cast<UVGDamageType_Slow>(DamageEvent.DamageTypeClass->GetDefaultObject()))
		{
			if (HasAuthority())
			{
				CharacterTags.AddTag(VigilantCharacter::Slowed);
                
				Multicast_ApplyDebuffMaterial(SlowDamageType->SlowDuration);
			}
		}
	}

	return ActualDamage;
}

void AVGCitizenCharacter::Multicast_ApplyDebuffMaterial_Implementation(float Duration)
{
	if (GetMesh() && RoarDebuffMaterial)
	{
		UMaterialInstanceDynamic* DebuffMID = UMaterialInstanceDynamic::Create(RoarDebuffMaterial, this);
		if (DebuffMID)
		{
			DebuffMID->SetScalarParameterValue(FName("StartTime"), GetWorld()->GetTimeSeconds());
			DebuffMID->SetScalarParameterValue(FName("Duration"), Duration);

			GetMesh()->SetOverlayMaterial(DebuffMID);
		}
	}

	GetWorldTimerManager().SetTimer(
		DebuffMaterialTimerHandle,
		this,
		&AVGCitizenCharacter::RemoveDebuffMaterial,
		Duration,
		false
	);
}

void AVGCitizenCharacter::RemoveDebuffMaterial()
{
	if (GetMesh())
	{
		GetMesh()->SetOverlayMaterial(nullptr);
	}

	if (HasAuthority())
	{
		CharacterTags.RemoveTag(VigilantCharacter::Slowed);
	}
}
void AVGCitizenCharacter::Hey()
{
	if (!HasAuthority()) 
	{
		Server_PlayHeySound();
		UE_LOG(LogTemp, Warning, TEXT("클라이언트 헤이 "));
	}
	else 
	{
		//리슨 서버(방장)인 경우 본인이 직접 인덱스를 뽑아서 방송
		if (CharacterDataAsset && CharacterDataAsset->HeySound.Num() > 0)
		{
			// 0부터 (배열크기 - 1) 사이의 랜덤한 숫자 추출
			int32 RandomIndex = FMath::RandRange(0, CharacterDataAsset->HeySound.Num() - 1);
			Multicast_PlayHeySound(RandomIndex);
		}
	}
}

void AVGCitizenCharacter::Multicast_PlayHeySound_Implementation(int32 SoundIndex)
{
	if (CharacterDataAsset && CharacterDataAsset->HeySound.IsValidIndex(SoundIndex))
	{
		USoundBase* SelectedSound = CharacterDataAsset->HeySound[SoundIndex];
		
		if (SelectedSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, SelectedSound, GetActorLocation());
		}
	}
	
	if (EmoteWidgetComponent)
	{
		EmoteWidgetComponent->SetVisibility(true);

		// 연타할 경우를 대비해 기존 타이머 초기화 후 재시작
		GetWorldTimerManager().ClearTimer(EmoteTimerHandle);
		GetWorldTimerManager().SetTimer(
			EmoteTimerHandle, 
			this, 
			&AVGCitizenCharacter::HideEmote, 
			1.0f, 
			false);
	}
}


void AVGCitizenCharacter::Server_PlayHeySound_Implementation()
{
	if (CharacterDataAsset && CharacterDataAsset->HeySound.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, CharacterDataAsset->HeySound.Num() - 1);
		UE_LOG(LogTemp, Warning, TEXT("서버 헤이 "));
		// 결정된 인덱스를 모든 클라이언트에게 전송
		Multicast_PlayHeySound(RandomIndex);
	}
}

void AVGCitizenCharacter::HideEmote()
{
	if (EmoteWidgetComponent)
	{
		EmoteWidgetComponent->SetVisibility(false);
	}
}