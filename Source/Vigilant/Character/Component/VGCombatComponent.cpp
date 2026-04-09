#include "Character/Component/VGCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "GameplayTagAssetInterface.h"
#include "VGEquipmentComponent.h"
#include "Common/VGGameplayTags.h"
#include "Data/VGWeaponDataAsset.h"
#include "Engine/Engine.h"
#include "Equipment/VGWeapon.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UVGCombatComponent::UVGCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UVGCombatComponent::SetActiveCombatData(UVGWeaponDataAsset* NewData)
{
	if (GetOwner()->HasAuthority())
	{
		ActiveCombatData = NewData;
	}
}

void UVGCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->OnMontageEnded.AddDynamic(this, &UVGCombatComponent::HandleMontageEnded);
		}
	}
}

void UVGCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UVGCombatComponent, ActiveCombatData);
	DOREPLIFETIME(UVGCombatComponent, CurrentCombatTags);
}

void UVGCombatComponent::OnRep_ActiveCombatData(UVGWeaponDataAsset* OldData)
{
	// TODO: 클라이언트 측에서 실행될 비주얼 관련 로직 작성 (e.g. UI 업데이트)
}

void UVGCombatComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	bool bWasAttackMontage = (Montage == Data->LightAttackMontage || Montage == Data->HeavyAttackMontage);
	if (!bWasAttackMontage)
	{
		return;
	}

	bCanChainCombo = false;
	bHasBufferedAttack = false;
	CurrentComboIndex = 0;

	HitActorsThisSwing.Empty();
	PreviousSocketLocations.Empty();
}

UVGWeaponDataAsset* UVGCombatComponent::GetCurrentCombatData() const
{
	return ActiveCombatData ? ActiveCombatData : DefaultCombatData;
}

// ---------------------------------------------------------
// INPUT & COMBO BUFFERING
// ---------------------------------------------------------

void UVGCombatComponent::TryLightAttack()
{
	if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(GetOwner()))
	{
		if (TagInterface->HasMatchingGameplayTag(VigilantCharacter::Dodge))
		{
			return;
		}
		if (!bCanChainCombo && TagInterface->HasMatchingGameplayTag(VigilantCharacter::Attacking))
		{
			return;
		}
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}
	
	// 1. 콤보 윈도우 안에 있다면: 버퍼가 인풋이 됨
	if (bCanChainCombo)
	{
		if (!bHasBufferedAttack)
		{
			bHasBufferedAttack = true;
			bIsBufferedAttackHeavy = false;
		}
		return;
	}
	
	// 2. 이미 공격중이고, 콤보 윈도우 안에 없다면: 공격 입력 무시됨
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (Data && OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_IsPlaying(Data->LightAttackMontage))
	{
		return;
	}

	// 3. 새로운 공격
	CurrentComboIndex = 0;
	PerformAttack(false);

	if (OwnerCharacter->IsLocallyControlled() && !OwnerCharacter->HasAuthority())
	{
		Server_TryAttack(false, CurrentComboIndex);
	}
}

void UVGCombatComponent::TryHeavyAttack()
{
	if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(GetOwner()))
	{
		if (TagInterface->HasMatchingGameplayTag(VigilantCharacter::Dodge))
		{
			return;
		}
		if (!bCanChainCombo && TagInterface->HasMatchingGameplayTag(VigilantCharacter::Attacking))
		{
			return;
		}
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (bCanChainCombo)
	{
		if (!bHasBufferedAttack)
		{
			bHasBufferedAttack = true;
			bIsBufferedAttackHeavy = true;
		}
		return;
	}

	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (Data && OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_IsPlaying(Data->HeavyAttackMontage))
	{
		return;
	}

	CurrentComboIndex = 0;
	PerformAttack(true);

	if (OwnerCharacter->IsLocallyControlled() && !OwnerCharacter->HasAuthority())
	{
		Server_TryAttack(true, CurrentComboIndex);
	}
}

void UVGCombatComponent::PerformAttack(bool bIsHeavy)
{
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	UAnimMontage* MontageToPlay = bIsHeavy ? Data->HeavyAttackMontage : Data->LightAttackMontage;
	if (!MontageToPlay)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	FString SectionPrefix = bIsHeavy ? TEXT("Heavy") : TEXT("Light");
	FName SectionName = FName(*FString::Printf(TEXT("%s%d"), *SectionPrefix, CurrentComboIndex + 1));

	if (!MontageToPlay->IsValidSectionName(SectionName))
	{
		bCanChainCombo = false;
		bHasBufferedAttack = false;
		CurrentComboIndex = 0;
		return;
	}

	if (AnimInstance->Montage_IsPlaying(MontageToPlay))
	{
		AnimInstance->Montage_JumpToSection(SectionName, MontageToPlay);
	}
	else
	{
		OwnerCharacter->PlayAnimMontage(MontageToPlay, Data->AttackSpeed, SectionName);
	}
}

// ---------------------------------------------------------
// ANIM NOTIFY COMBO HOOKS
// ---------------------------------------------------------

void UVGCombatComponent::OnComboWindowOpened()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	bCanChainCombo = true;
	bHasBufferedAttack = false;
}

void UVGCombatComponent::OnComboWindowClosed()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	bCanChainCombo = false;

	if (bHasBufferedAttack)
	{
		CurrentComboIndex++;
		bHasBufferedAttack = false;

		PerformAttack(bIsBufferedAttackHeavy);

		if (!OwnerPawn->HasAuthority())
		{
			Server_TryAttack(bIsBufferedAttackHeavy, CurrentComboIndex);
		}
	}
	else
	{
		CurrentComboIndex = 0;
	}
}

// ---------------------------------------------------------
// NETWORKING RPCS
// ---------------------------------------------------------


void UVGCombatComponent::Server_TryAttack_Implementation(bool bIsHeavy, int32 ExpectedComboIndex)
{
	CurrentComboIndex = ExpectedComboIndex;
	PerformAttack(bIsHeavy);

	// Multicast
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (Data)
	{
		UAnimMontage* MontageToPlay = bIsHeavy ? Data->HeavyAttackMontage : Data->LightAttackMontage;
		FString SectionPrefix = bIsHeavy ? TEXT("Heavy") : TEXT("Light");
		FName SectionName = FName(*FString::Printf(TEXT("%s%d"), *SectionPrefix, CurrentComboIndex + 1));

		Multicast_PlayAttackMontage(MontageToPlay, SectionName, Data->AttackSpeed);
	}
}

bool UVGCombatComponent::Server_TryAttack_Validate(bool bIsHeavy, int32 ExpectedComboIndex)
{
	return true;
}

void UVGCombatComponent::Client_CancelAttackPrediction_Implementation()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerCharacter->StopAnimMontage();
	}
}


void UVGCombatComponent::Multicast_PlayAttackMontage_Implementation(UAnimMontage* MontageToPlay, FName SectionName,
                                                                    float PlayRate)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	if (OwnerPawn->IsLocallyControlled() || (OwnerPawn->HasAuthority() && IsNetMode(NM_DedicatedServer)))
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerPawn);
	if (OwnerCharacter)
	{
		OwnerCharacter->PlayAnimMontage(MontageToPlay, PlayRate, SectionName);
	}
}

// ---------------------------------------------------------
// HIT DETECTION
// ---------------------------------------------------------


void UVGCombatComponent::StartMeleeTrace()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	HitActorsThisSwing.Empty();
	PreviousSocketLocations.Empty();

	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	// --- Test ---
	UMeshComponent* TraceMesh = OwnerCharacter->GetMesh();

	if (UVGEquipmentComponent* EquipComp = OwnerCharacter->FindComponentByClass<UVGEquipmentComponent>())
	{
		if (AVGWeapon* EquippedWeapon = Cast<AVGWeapon>(EquipComp->RightHandItem))
		{
			if (UMeshComponent* WeaponMesh = EquippedWeapon->GetWeaponMesh())
			{
				TraceMesh = WeaponMesh;
			}
		}
	}

	if (!TraceMesh)
	{
		return;
	}

	// Data Asset에 정의된 모든 소켓의 시작 위치 기록
	for (const FName& SocketName : Data->HitboxSocketNames)
	{
		FVector StartLoc = TraceMesh->GetSocketLocation(SocketName);
		PreviousSocketLocations.Add(SocketName, StartLoc);
	}
}

void UVGCombatComponent::TickMeleeTrace()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	// --- Test ---
	// 트레이스할 메시 결정: 기본값 주먹
	UMeshComponent* TraceMesh = OwnerCharacter->GetMesh();

	UVGEquipmentComponent* EquipComp = OwnerCharacter->FindComponentByClass<UVGEquipmentComponent>();
	if (EquipComp && EquipComp->RightHandItem)
	{
		if (AVGWeapon* EquippedWeapon = Cast<AVGWeapon>(EquipComp->RightHandItem))
		{
			if (EquippedWeapon->GetWeaponMesh())
			{
				TraceMesh = EquippedWeapon->GetWeaponMesh();
			}
		}
	}
	// ---

	if (!TraceMesh)
	{
		return;
	}

	float AttackRadius = 20.0f;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	// QueryParams.bTraceComplex = true;

	for (const FName& SocketName : Data->HitboxSocketNames)
	{
		if (!PreviousSocketLocations.Contains(SocketName))
		{
			continue;
		}

		FVector PreviousLoc = PreviousSocketLocations[SocketName];
		FVector CurrentLoc = TraceMesh->GetSocketLocation(SocketName);

		TArray<FHitResult> HitResults;

		bool bHit = GetWorld()->SweepMultiByChannel(
			HitResults,
			PreviousLoc,
			CurrentLoc,
			FQuat::Identity,
			ECC_Pawn,
			Sphere,
			QueryParams
		);

		// --- Debug ---
		FVector TraceVector = CurrentLoc - PreviousLoc;
		float TraceLength = TraceVector.Size();
		if (TraceLength > KINDA_SMALL_NUMBER)
		{
			FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();
			DrawDebugCapsule(
				GetWorld(),
				PreviousLoc + (TraceVector * 0.5f),
				(TraceLength * 0.5f) + AttackRadius,
				AttackRadius,
				CapsuleRot,
				bHit ? FColor::Green : FColor::Red,
				false,
				2.0f
			);
		}
		// ---

		if (bHit)
		{
			for (const FHitResult& HitResult : HitResults)
			{
				AActor* HitActor = HitResult.GetActor();
				if (HitActor && !HitActorsThisSwing.Contains(HitActor))
				{
					HitActorsThisSwing.Add(HitActor);
					Server_ProcessHit(HitActor);
				}
			}
		}

		PreviousSocketLocations[SocketName] = CurrentLoc;
	}
}

void UVGCombatComponent::StopMeleeTrace()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	HitActorsThisSwing.Empty();
}


void UVGCombatComponent::Server_ProcessHit_Implementation(AActor* HitActor)
{
	AActor* Owner = GetOwner();
	UVGWeaponDataAsset* Data = GetCurrentCombatData();

	if (!Owner || !Data || !HitActor)
	{
		return;
	}

	// Validation
	float Distance = FVector::Distance(Owner->GetActorLocation(), HitActor->GetActorLocation());
	float MaxAllowedDistance = 300.0f;

	if (Distance <= MaxAllowedDistance)
	{
		UGameplayStatics::ApplyDamage
		(
			HitActor,
			Data->BaseDamage,
			Owner->GetInstigatorController(),
			Owner,
			nullptr
		);
	}
}

bool UVGCombatComponent::Server_ProcessHit_Validate(AActor* HitActor)
{
	return true;
}
