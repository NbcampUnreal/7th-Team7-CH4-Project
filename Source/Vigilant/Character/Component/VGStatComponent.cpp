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
	DOREPLIFETIME(UVGStatComponent, bIsAlive);
	
	//현진 : 추후 스테미나 관련 기획이 생겨 다른 플레이어들 에게도 스테미너 상태를 알려야 한다면 변경필요
	DOREPLIFETIME_CONDITION(UVGStatComponent, CurrentStamina, COND_OwnerOnly);
}

void UVGStatComponent::ApplyDamage(float DamageAmount)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	if (!bIsAlive)
	{
		return;
	}

	// [Fix] KINDA_SMALL_NUMBER(~0.0001) → 0.f — UI에서 HP가 0이 아닌 미세값으로 표시되는 문제 방지
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

	if (CurrentHP <= 0.f && bIsAlive)
	{
		bIsAlive = false;
		// [Fix] 사망 시 스태미나 재생 타이머 정리 — 죽은 캐릭터가 계속 회복하는 것 방지
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
		OnDead.Broadcast();
	}
	
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
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

	// [Fix] KINDA_SMALL_NUMBER → 0.f — ApplyDamage와 동일하게 통일
	CurrentHP = FMath::Clamp(CurrentHP + RecoverAmount, 0.f, MaxHP);
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void UVGStatComponent::ConsumeStamina(float ConsumeAmount)
{
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

void UVGStatComponent::ResetStats()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
	
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;
	bIsAlive = true;
	
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UVGStatComponent::OnRep_bIsAlive()
{
	if (!bIsAlive)
	{
		OnDead.Broadcast();
	}
}

void UVGStatComponent::OnRep_CurrentHP()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void UVGStatComponent::OnRep_CurrentStamina()
{
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}