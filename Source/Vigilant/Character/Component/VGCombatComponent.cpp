#include "Character/Component/VGCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "GameplayTagAssetInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "VGStatComponent.h"
#include "Combat/VGAmmoProviderInterface.h"
#include "Combat/VGAttackExecution.h"
#include "Combat/VGProjectile.h"
#include "Data/VGShieldDataAsset.h"
#include "Data/VGWeaponDataAsset.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UVGCombatComponent::UVGCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UVGCombatComponent::SetActiveCombatData(UVGWeaponDataAsset* NewData, UMeshComponent* NewTraceMesh)
{
	if (GetOwner()->HasAuthority())
	{
		ActiveCombatData = NewData;
	}

	ActiveTraceMesh = NewTraceMesh;
	InstantiateExecutionObject();
}

void UVGCombatComponent::SetActiveShieldData(UVGShieldDataAsset* NewData)
{
	if (GetOwner()->HasAuthority())
	{
		ActiveShieldData = NewData;
	}
}

void UVGCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	InstantiateExecutionObject();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		CachedStatComponent = OwnerCharacter->FindComponentByClass<UVGStatComponent>();

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
	DOREPLIFETIME(UVGCombatComponent, ActiveShieldData);
	DOREPLIFETIME(UVGCombatComponent, CurrentCombatTags);
}

void UVGCombatComponent::OnRep_ActiveCombatData(UVGWeaponDataAsset* OldData)
{
	InstantiateExecutionObject();
	// TODO: 클라이언트 측에서 실행될 비주얼 관련 로직 작성 (e.g. UI 업데이트)
}

void UVGCombatComponent::OnRep_ActiveShieldData(UVGShieldDataAsset* OldData)
{
	// TODO: 클라이언트 측에서 실행될 비주얼 관련 로직 작성 (e.g. UI 업데이트)
}

void UVGCombatComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveAttackMontage)
	{
		return;
	}

	ActiveAttackMontage = nullptr;
	bCanChainCombo = false;
	bHasBufferedAttack = false;
	CurrentComboIndex = 0;

	SetCombatRotationMode(false);

	if (CurrentExecution)
	{
		CurrentExecution->StopAttack();
	}
}

void UVGCombatComponent::InstantiateExecutionObject()
{
	CurrentExecution = nullptr;
	UVGWeaponDataAsset* Data = GetCurrentCombatData(); // Active 또는 Default

	if (Data && Data->AttackExecutionTemplate)
	{
		CurrentExecution = DuplicateObject(Data->AttackExecutionTemplate, this);
		CurrentExecution->Initialize(this);
	}
}

void UVGCombatComponent::SetCombatRotationMode(bool bIsAttacking)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (bIsAttacking)
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = true;
	}
	else
	{
		MovementComponent->bUseControllerDesiredRotation = false;
		MovementComponent->bOrientRotationToMovement = true;
	}
}

UVGWeaponDataAsset* UVGCombatComponent::GetCurrentCombatData() const
{
	return ActiveCombatData ? ActiveCombatData : DefaultCombatData;
}

UVGShieldDataAsset* UVGCombatComponent::GetCurrentShieldData() const
{
	return ActiveShieldData ? ActiveShieldData : nullptr;
}

UMeshComponent* UVGCombatComponent::GetActiveTraceMesh() const
{
	return ActiveTraceMesh.IsValid() ? ActiveTraceMesh.Get() : nullptr;
}

void UVGCombatComponent::SetDamageMultiplier(float NewMultiplier)
{
	if (GetOwner()->HasAuthority())
	{
		DamageMultiplier = NewMultiplier;
	}
}

// ---------------------------------------------------------
// INPUT & COMBO BUFFERING
// ---------------------------------------------------------

void UVGCombatComponent::TryLightAttack()
{
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

	// 스태미나 확인
	if (CachedStatComponent.IsValid())
	{
		if (CachedStatComponent->GetCurrentStamina() < Data->LightAttackStaminaCost)
		{
			return;
		}
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
	if (!Data || !Data->HeavyAttackMontage)
	{
		return;
	}

	if (Data && OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_IsPlaying(Data->HeavyAttackMontage))
	{
		return;
	}

	if (CachedStatComponent.IsValid())
	{
		if (CachedStatComponent->GetCurrentStamina() < Data->LightAttackStaminaCost)
		{
			return;
		}
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
	bIsCurrentAttackHeavy = bIsHeavy;

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
		if (Data->bFaceCameraDuringAttack)
		{
			SetCombatRotationMode(true);
		}
	}

	ActiveAttackMontage = MontageToPlay;
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
	// --- 0. Stamina Check & Consume ---
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	if (CachedStatComponent.IsValid())
	{
		float RequiredStamina = bIsHeavy ? Data->HeavyAttackStaminaCost : Data->LightAttackStaminaCost;
		if (CachedStatComponent->GetCurrentStamina() < RequiredStamina)
		{
			Client_CancelAttackPrediction();
			return;
		}

		CachedStatComponent->ConsumeStamina(RequiredStamina);
	}

	// --- 1. Ammo Validation ---
	if (UMeshComponent* TraceMesh = GetActiveTraceMesh())
	{
		if (IVGAmmoProviderInterface* AmmoProvider = Cast<IVGAmmoProviderInterface>(TraceMesh->GetOwner()))
		{
			if (!AmmoProvider->HasAmmo())
			{
				Client_CancelAttackPrediction();
				return;
			}
		}
	}

	// --- 2. Attack Execution ---
	CurrentComboIndex = ExpectedComboIndex;
	PerformAttack(bIsHeavy);

	// Multicast
	UAnimMontage* MontageToPlay = bIsHeavy ? Data->HeavyAttackMontage : Data->LightAttackMontage;
	FString SectionPrefix = bIsHeavy ? TEXT("Heavy") : TEXT("Light");
	FName SectionName = FName(*FString::Printf(TEXT("%s%d"), *SectionPrefix, CurrentComboIndex + 1));

	Multicast_PlayAttackMontage(MontageToPlay, SectionName, Data->AttackSpeed);
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


void UVGCombatComponent::Server_ProcessHit_Implementation(const FHitResult& HitResult)
{
	AActor* Owner = GetOwner();
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	AActor* HitActor = HitResult.GetActor();

	if (!Owner || !Data || !HitActor)
	{
		return;
	}

	// Validation
	float Distance = FVector::Distance(Owner->GetActorLocation(), HitActor->GetActorLocation());
	if (Distance <= Data->MaxAttackRange)
	{
		float AttackDamage = bIsCurrentAttackHeavy ? Data->HeavyAttackDamage : Data->LightAttackDamage;
		// (이용호 추가) 데미지 배율 계산용
		float FinalDamage = AttackDamage * DamageMultiplier;

		UGameplayStatics::ApplyPointDamage
		(
			HitActor,
			FinalDamage,
			HitResult.ImpactNormal,
			HitResult,
			Owner->GetInstigatorController(),
			Owner,
			nullptr
		);
		
		Multicast_PlayImpactFeedback(HitResult, Data);
	}
}

bool UVGCombatComponent::Server_ProcessHit_Validate(const FHitResult& HitResult)
{
	return true;
}

void UVGCombatComponent::Server_SpawnProjectile_Implementation(TSubclassOf<AActor> ProjectileClass,
                                                               const FVector& SpawnLocation,
                                                               const FRotator& SpawnRotation)
{
	if (!ProjectileClass)
	{
		return;
	}

	// --- 1. Ammo Validation ---
	IVGAmmoProviderInterface* AmmoProvider = nullptr;
	if (UMeshComponent* TraceMesh = GetActiveTraceMesh())
	{
		AmmoProvider = Cast<IVGAmmoProviderInterface>(TraceMesh->GetOwner());
		if (AmmoProvider && !AmmoProvider->HasAmmo())
		{
			Client_CancelAttackPrediction();
			return;
		}
	}

	// --- 2. Load Data ---
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	// --- 3. Spawn Projectile ---
	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedActor = World->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (AVGProjectile* Projectile = Cast<AVGProjectile>(SpawnedActor))
		{
			float AttackDamage = Data->LightAttackDamage;
			// (이용호 추가) 데미지 배율 계산용
			float FinalDamage = AttackDamage * DamageMultiplier;
			Projectile->InitializeProjectile(FinalDamage);
		}
	}

	// --- 4. Consume Ammo ---
	if (AmmoProvider)
	{
		AmmoProvider->ConsumeAmmo();
	}
}

bool UVGCombatComponent::Server_SpawnProjectile_Validate(TSubclassOf<AActor> ProjectileClass,
                                                         const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	return ProjectileClass != nullptr;
}

void UVGCombatComponent::Multicast_PlayImpactFeedback_Implementation(const FHitResult& HitResult,
																	 UVGWeaponDataAsset* WeaponData)
{
	if (!WeaponData)
	{
		return;
	}

	// 1. 타격 사운드 재생
	if (WeaponData->ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->ImpactSound, HitResult.ImpactPoint);
	}

	// 2. VFX 재생
	if (WeaponData->ImpactVFX)
	{
		FRotator VFXRotation = HitResult.ImpactNormal.Rotation();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, WeaponData->ImpactVFX, HitResult.ImpactPoint, VFXRotation);
	}
	
	// 3. 더미 볼트 생성
	if (WeaponData->DummyActorClass && HitResult.GetComponent())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		FRotator DummyRotation = HitResult.ImpactNormal.Rotation();
		DummyRotation.Pitch -= 180.0f;
		
		if (AActor* Dummy = GetWorld()->SpawnActor<AActor>(WeaponData->DummyActorClass, HitResult.ImpactPoint, DummyRotation, SpawnParams))
		{
			Dummy->AttachToComponent(HitResult.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
			Dummy->SetLifeSpan(10.0f);
		}
	}
}

// ---------------------------------------------------------
// Attack Execution
// ---------------------------------------------------------


void UVGCombatComponent::StartAttackExecution()
{
	if (CurrentExecution)
	{
		CurrentExecution->StartAttack();
	}
}

void UVGCombatComponent::TickAttackExecution()
{
	if (CurrentExecution)
	{
		CurrentExecution->TickAttack();
	}
}

void UVGCombatComponent::StopAttackExecution()
{
	if (CurrentExecution)
	{
		CurrentExecution->StopAttack();
	}
}

// ---------------------------------------------------------
// Blocking
// ---------------------------------------------------------

void UVGCombatComponent::TryStartBlock()
{
	UVGShieldDataAsset* Data = GetCurrentShieldData();

	if (!Data || !Data->BlockMontage)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance && AnimInstance->Montage_IsPlaying(Data->BlockMontage))
		{
			return;
		}

		OwnerCharacter->PlayAnimMontage(Data->BlockMontage);
		if (OwnerCharacter->IsLocallyControlled() && !OwnerCharacter->HasAuthority())
		{
			Server_StartBlock();
		}
	}
}

void UVGCombatComponent::TryStopBlock()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	UVGShieldDataAsset* Data = GetCurrentShieldData();

	if (OwnerCharacter && Data && Data->BlockMontage)
	{
		OwnerCharacter->StopAnimMontage(Data->BlockMontage);
		if (OwnerCharacter->IsLocallyControlled() && !OwnerCharacter->HasAuthority())
		{
			Server_StopBlock();
		}
	}
}

void UVGCombatComponent::Server_StartBlock_Implementation()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	UVGShieldDataAsset* Data = GetCurrentShieldData();

	if (OwnerCharacter && Data && Data->BlockMontage)
	{
		OwnerCharacter->PlayAnimMontage(Data->BlockMontage);
		Multicast_PlayBlockMontage(Data->BlockMontage);
		OnGuardStateChanged.Broadcast(true);
	}
}

bool UVGCombatComponent::Server_StartBlock_Validate()
{
	return true;
}


void UVGCombatComponent::Server_StopBlock_Implementation()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	UVGShieldDataAsset* Data = GetCurrentShieldData();

	if (OwnerCharacter && Data && Data->BlockMontage)
	{
		OwnerCharacter->StopAnimMontage(Data->BlockMontage);
		Multicast_StopBlockMontage(Data->BlockMontage);
		OnGuardStateChanged.Broadcast(false);
	}
}

bool UVGCombatComponent::Server_StopBlock_Validate()
{
	return true;
}


void UVGCombatComponent::Multicast_PlayBlockMontage_Implementation(UAnimMontage* MontageToPlay)
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
		OwnerCharacter->PlayAnimMontage(MontageToPlay);
	}
}

void UVGCombatComponent::Multicast_StopBlockMontage_Implementation(UAnimMontage* MontageToStop)
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
		OwnerCharacter->StopAnimMontage(MontageToStop);
	}
}

void UVGCombatComponent::Multicast_PlayShieldFeedback_Implementation(bool bIsParry, FVector ImpactLocation,
	FRotator ImpactRotation)
{
	UVGShieldDataAsset* ShieldData = GetCurrentShieldData();
	if (!ShieldData)
	{
		return;
	}
	
	USoundBase* SoundToPlay = bIsParry ? ShieldData->ParrySound : ShieldData->BlockSound;
	UNiagaraSystem* VFXToPlay = bIsParry ? ShieldData->ParryVFX : ShieldData->BlockVFX;
	
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, ImpactLocation);
	}
	
	if (VFXToPlay)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, VFXToPlay, ImpactLocation, ImpactRotation);
	}
}
