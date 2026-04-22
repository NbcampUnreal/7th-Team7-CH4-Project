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
}

void UVGStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVGStatComponent, CurrentHP);
	DOREPLIFETIME(UVGStatComponent, MaxHP);
	DOREPLIFETIME(UVGStatComponent, bIsAlive);
	DOREPLIFETIME(UVGStatComponent, LastInstigator);
	
	//현진 : 추후 스테미나 관련 기획이 생겨 다른 플레이어들 에게도 스테미너 상태를 알려야 한다면 변경필요
	DOREPLIFETIME_CONDITION(UVGStatComponent, CurrentStamina, COND_OwnerOnly);
}

void UVGStatComponent::ApplyDamageToStat(float DamageAmount, AController* Instigator)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	if (!bIsAlive || DamageAmount <= KINDA_SMALL_NUMBER)
	{
		return;
	}
	LastInstigator = Instigator;
	
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);
	
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	
	UE_LOG
	(	
		LogTemp,
		Warning,
		TEXT("[%s] Take Damage! / DamageAmount: %.1f / Instigator: %s / CurrentHP: %.1f"), 
		*GetOwner()->GetName(), 
		DamageAmount, 
		Instigator ? *Instigator->GetName() : TEXT("Unknown"), 
		CurrentHP
	);
	
	if (CurrentHP <= 0.f && bIsAlive)
	{
		bIsAlive = false;
		
		UE_LOG
		(	
			LogTemp,
			Error,
			TEXT("[%s] Dead! / LastInstigator: %s"), 
			*GetOwner()->GetName(), 
			Instigator ? *Instigator->GetName() : TEXT("Unknown")
		);
		
		OnDead.Broadcast(LastInstigator);
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
	
	if (GetWorld()->GetTimerManager().IsTimerActive(StaminaContinuousConsumeTimerHandle))
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
	LastInstigator = nullptr;
	ContinuousConsumeRate = 0.f;
	
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UVGStatComponent::OnRep_bIsAlive()
{
	if (!bIsAlive)
	{
		OnDead.Broadcast(LastInstigator);
	}
}

void UVGStatComponent::OnRep_MaxHP()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void UVGStatComponent::OnRep_CurrentHP(float OldHP)
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void UVGStatComponent::OnRep_CurrentStamina()
{
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UVGStatComponent::InitStat(float InMaxHP, float InMaxStamina)
{
	// 스탯 초기화는 서버에서만 수행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	MaxHP = InMaxHP;
	MaxStamina = InMaxStamina;

	// 최대치 갱신 후 현재 스탯도 꽉 채움
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;
	bIsAlive = true;

	// 변경된 스탯을 UI 및 다른 컴포넌트에 알림
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}