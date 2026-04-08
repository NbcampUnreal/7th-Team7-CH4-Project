// Fill out your copyright notice in the Description page of Project Settings.


#include "VGStatComponent.h"
#include "Net/UnrealNetwork.h"

UVGStatComponent::UVGStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UVGStatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;
	
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UVGStatComponent::TakeDamage);
	}
}

void UVGStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVGStatComponent, CurrentHP);
	DOREPLIFETIME(UVGStatComponent, bIsAlive);
	DOREPLIFETIME(UVGStatComponent, LastDamageCauser);
	
	//현진 : 추후 스테미나 관련 기획이 생겨 다른 플레이어들 에게도 스테미너 상태를 알려야 한다면 변경필요
	DOREPLIFETIME_CONDITION(UVGStatComponent, CurrentStamina, COND_OwnerOnly);
}

void UVGStatComponent::TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	if (!bIsAlive || Damage <= KINDA_SMALL_NUMBER)
	{
		return;
	}
	LastDamageCauser = DamageCauser;
	
	CurrentHP = FMath::Clamp(CurrentHP - Damage, 0.f, MaxHP);
	
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	
	if (CurrentHP <= 0.f && bIsAlive)
	{
		bIsAlive = false;
		
		OnDead.Broadcast(LastDamageCauser);
	}
}

void UVGStatComponent::RecoverHP(float RecoverAmount)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	if (!bIsAlive)
	{
		return;
	}

	CurrentHP = FMath::Clamp(CurrentHP + RecoverAmount, 0.f, MaxHP);
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void UVGStatComponent::ConsumeStamina(float ConsumeAmount)
{
	if (!bIsAlive)
	{
		return;
	}
	
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	CurrentStamina = FMath::Clamp(CurrentStamina - ConsumeAmount, 0.f, MaxStamina);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	
	StartStaminaRegenTimer();
}

void UVGStatComponent::StartStaminaRegenTimer()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
	
	GetWorld()->GetTimerManager().SetTimer(
		StaminaRegenTimerHandle, 
		this, 
		&UVGStatComponent::RegenerateStamina, 
		StaminaRegenInterval, 
		true, 
		StaminaRegenDelay
	);
}

void UVGStatComponent::RegenerateStamina()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	if (CurrentStamina < MaxStamina)
	{
		float RegenAmount = StaminaRegenRate * StaminaRegenInterval;
		CurrentStamina = FMath::Clamp(CurrentStamina + RegenAmount, 0.f, MaxStamina);
        
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
	}
}

void UVGStatComponent::RecoverStamina(float RecoverAmount)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	CurrentStamina = FMath::Clamp(CurrentStamina + RecoverAmount, 0.f, MaxStamina);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UVGStatComponent::StartContinuousConsumeStamina(float ConsumeAmountPerSecond)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	if (FMath::IsNearlyEqual(ContinuousConsumeRate, ConsumeAmountPerSecond))
	{
		return;
	}

	ContinuousConsumeRate = ConsumeAmountPerSecond;

	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(StaminaContinuousConsumeTimerHandle);
	
	GetWorld()->GetTimerManager().SetTimer(
		StaminaContinuousConsumeTimerHandle,
		this,
		&UVGStatComponent::UseStaminaTick,
		StaminaConsumeInterval,
		true
	);
}

void UVGStatComponent::StopContinuousConsumeStamina()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	

	GetWorld()->GetTimerManager().ClearTimer(StaminaContinuousConsumeTimerHandle);
	ContinuousConsumeRate = 0.f;

	StartStaminaRegenTimer();
}

void UVGStatComponent::UseStaminaTick()
{
	if (!bIsAlive)
	{
		return;
	}
	
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	float ConsumeAmount = ContinuousConsumeRate * StaminaConsumeInterval;
	CurrentStamina = FMath::Clamp(CurrentStamina - ConsumeAmount, 0.f, MaxStamina);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

	if (CurrentStamina <= 0.f)
	{
		StopContinuousConsumeStamina();
	}
}

void UVGStatComponent::ResetStats()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(StaminaContinuousConsumeTimerHandle);
	
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;
	bIsAlive = true;
	LastDamageCauser = nullptr;
	ContinuousConsumeRate = 0.f;
	
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UVGStatComponent::OnRep_bIsAlive()
{
	if (!bIsAlive)
	{
		OnDead.Broadcast(LastDamageCauser);
	}
}

void UVGStatComponent::OnRep_CurrentHP(float OldHP)
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void UVGStatComponent::OnRep_CurrentStamina()
{
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}