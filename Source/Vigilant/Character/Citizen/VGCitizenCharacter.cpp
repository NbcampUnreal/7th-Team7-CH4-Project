// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Citizen/VGCitizenCharacter.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Equipment/VGEquippableActor.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Common/VGGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Interfaces GameplayTag
void AVGCitizenCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = CharacterTags;
}

void AVGCitizenCharacter::AddGameplayTag(FGameplayTag TagToAdd)
{
	CharacterTags.AddTag(TagToAdd);
}

void AVGCitizenCharacter::RemoveGameplayTag(FGameplayTag TagToRemove)
{
	CharacterTags.RemoveTag(TagToRemove);
}
#pragma endregion

AVGCitizenCharacter::AVGCitizenCharacter()
{
	//속도 조정
	NormalSpeed = 320.f;
	SprintSpeed = 500.f;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	
	OriginalFriction = GetCharacterMovement()->GroundFriction;
	ModifyFriction = 2.f;
	// 장비 컴포넌트 생성
	EquipmentComponent = CreateDefaultSubobject<UVGEquipmentComponent>(TEXT("EquipmentComponent"));
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
	}
}

void AVGCitizenCharacter::Interact()
{
	if (!IsLocallyControlled()) return;
    if (FollowCamera == nullptr || EquipmentComponent == nullptr) return;

    FVector StartLocation = FollowCamera->GetComponentLocation();
    FVector ForwardVector = FollowCamera->GetForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * 1500.0f);

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);

    FCollisionShape SphereShape = FCollisionShape::MakeSphere(30.0f);

    bool bHit = GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::Identity, ECC_Visibility, SphereShape, CollisionParams);

    // 디버그 그리기
    DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f);
    if (bHit)
        DrawDebugSphere(GetWorld(), HitResult.Location, 30.0f, 16, FColor::Green, false, 2.0f);
    else
        DrawDebugSphere(GetWorld(), EndLocation, 30.0f, 16, FColor::Red, false, 2.0f);

    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();
        if (AVGEquippableActor* EquippableItem = Cast<AVGEquippableActor>(HitActor))
        {
            EVGEquipmentType TypeToEquip = EVGEquipmentType::Weapon;
            if (EquippableItem->GetName().Contains("Mission"))
            {
                TypeToEquip = EVGEquipmentType::MissionItem;
            }

            EquipmentComponent->Server_EquipItem(EquippableItem, TypeToEquip);

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
        }
    }
}

void AVGCitizenCharacter::DropItem()
{
	if (EquipmentComponent)
	{
		EquipmentComponent->Server_DropItem(ActiveEquipmentSlot);
		UE_LOG(LogTemp, Log, TEXT("현재 활성화된 슬롯의 아이템 버리기"));
	}
}

void AVGCitizenCharacter::SelectSlot(const FInputActionValue& Value)
{
	float SlotNumber = Value.Get<float>();

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

void AVGCitizenCharacter::Move(const FInputActionValue& Value)
{
	if (CharacterTags.HasTag(VigilantCharacter::Dodge))
	{
		//구르기상태는 이동불가
		return;
	}
	
	Super::Move(Value);
	
}

void AVGCitizenCharacter::Dodge()
{
	// 특정 태그 보유시 리턴, 추가가능
	if (CharacterTags.HasTag(VigilantCharacter::Dodge))
	{
		return;
	}

	CharacterTags.AddTag(VigilantCharacter::Dodge);
	//방향 계산
	FVector DodgeDirection = GetCharacterMovement()->GetLastInputVector();
	if(DodgeDirection.IsNearlyZero())
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

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(DodgeAnimation);

		FOnMontageBlendingOutStarted BlendingOutStarted;
		BlendingOutStarted.BindUObject(this, &AVGCitizenCharacter::OnMontageCompleted);
		AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutStarted, DodgeAnimation);
		
		//루트모션이 아닌 직접 날리자.. 멀티플레이상황에서는 루트모션이 버벅거림
	
		
		FVector DodgeVelocity = Direction*DodgeForce;
		DodgeVelocity.Z = DodgeZForce;
		LaunchCharacter(DodgeVelocity,true,true);
		
		FRotator DodgeRotattion = Direction.Rotation();
		DodgeRotattion.Pitch = 0.f;
		DodgeRotattion.Roll = 0.f;
		SetActorRotation(DodgeRotattion);
		//구르기 느낌을 위한 마찰력 조절
		GetCharacterMovement()->GroundFriction = ModifyFriction;
		GetCharacterMovement()->GroundFriction;
	}
}

void AVGCitizenCharacter::Multicast_Dodge_Implementation()
{
	//다른 클라이언트에서는 애니매이션만 재생
	if (!IsLocallyControlled()&&!HasAuthority()) // 서버도 로컬도 아닐때, 즉 이 액터가 Simulated일때
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


void AVGCitizenCharacter::OnMontageCompleted(UAnimMontage* Montage, bool bWasCancelled)
{
	
	CharacterTags.RemoveTag(VigilantCharacter::Dodge);
	GetCharacterMovement()->GroundFriction = OriginalFriction;
	if (bWasCancelled == true)
	{
		//회피가 불명의 이유로 중단되었을때 로직
		
	}
	else
	{
		//회피가 잘 끝났을 때 로직
	}
}
