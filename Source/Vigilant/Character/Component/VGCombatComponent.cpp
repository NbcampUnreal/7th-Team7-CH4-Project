#include "Character/Component/VGCombatComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "Data/VGWeaponDataAsset.h"
#include "GameFramework/Character.h"

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

UVGWeaponDataAsset* UVGCombatComponent::GetCurrentCombatData() const
{
	return ActiveCombatData ? ActiveCombatData : DefaultCombatData;
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
	// TODO: bIsHeavy = true로 구현
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
	if (OwnerCharacter)
	{
		FString SectionPrefix = bIsHeavy ? TEXT("Heavy") : TEXT("Light");
		FName SectionName = FName(*FString::Printf(TEXT("%s%d"), *SectionPrefix, CurrentComboIndex + 1));
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
	UVGWeaponDataAsset *Data = GetCurrentCombatData();
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
	// TODO: 구현 예정
}

void UVGCombatComponent::TickMeleeTrace()
{
}

void UVGCombatComponent::StopMeleeTrace()
{
	//  TODO: 구현 예정
}
