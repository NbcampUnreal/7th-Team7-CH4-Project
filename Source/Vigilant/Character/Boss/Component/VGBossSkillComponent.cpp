// Fill out your copyright notice in the Description page of Project Settings.


#include "VGBossSkillComponent.h"
#include "Common/VGGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"

UVGBossSkillComponent::UVGBossSkillComponent()
{
	// [Fix] Tick 미사용 — 불필요한 Tick 비용 제거
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}


void UVGBossSkillComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UVGBossSkillComponent::ExecuteSkill_Q()
{
	Server_ExecuteSkill_Q();
}

void UVGBossSkillComponent::Server_ExecuteSkill_Q_Implementation()
{
	// [서버] 태그 및 쿨타임 검사
	if (ActiveStateTags.HasTag(VigilantBoss::Casting) || ActiveStateTags.HasTag(VigilantBoss::SkillCooldown_Q)) 
	{
		return;
	}

	// [서버] 쿨타임 및 시전 상태 돌입
	ActiveStateTags.AddTag(VigilantBoss::Casting);
	ActiveStateTags.AddTag(VigilantBoss::SkillCooldown_Q);
    
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Q, this, &UVGBossSkillComponent::ResetCooldown_Q, CooldownTime_Q, false);

	UE_LOG(LogTemp, Warning, TEXT("[VGBossSkill - Server] Q 스킬 시전 - 쿨타임 %f초"), CooldownTime_Q);

	// [서버 -> 모두에게] 모션 재생 지시
	Multicast_ExecuteSkill_Q();
}

void UVGBossSkillComponent::Multicast_ExecuteSkill_Q_Implementation()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && SkillMontage_Q)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);

		// [Fix] GetMesh() null 체크 추가 — 메시 미설정 시 크래시 방지
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(SkillMontage_Q);
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &UVGBossSkillComponent::OnSkillMontageEnded);
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, SkillMontage_Q);
		}
	}
}

void UVGBossSkillComponent::ExecuteSkill_E()
{
	Server_ExecuteSkill_E();
}

void UVGBossSkillComponent::Server_ExecuteSkill_E_Implementation()
{
	// [서버] 태그 및 쿨타임 검사
	if (ActiveStateTags.HasTag(VigilantBoss::Casting) || ActiveStateTags.HasTag(VigilantBoss::SkillCooldown_E)) 
	{
		return;
	}

	// [서버] 쿨타임 및 시전 상태 돌입
	ActiveStateTags.AddTag(VigilantBoss::Casting);
	ActiveStateTags.AddTag(VigilantBoss::SkillCooldown_E);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_E, this, &UVGBossSkillComponent::ResetCooldown_E, CooldownTime_E, false);

	UE_LOG(LogTemp, Warning, TEXT("[VGBossSkill - Server] E 스킬 시전 - 쿨타임 %f초"), CooldownTime_E);

	// [서버 -> 모두에게] 모션 재생 지시
	Multicast_ExecuteSkill_E();
}

void UVGBossSkillComponent::Multicast_ExecuteSkill_E_Implementation()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && SkillMontage_E)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);

		// [Fix] GetMesh() null 체크 추가
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(SkillMontage_E);
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &UVGBossSkillComponent::OnSkillMontageEnded);
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, SkillMontage_E);
		}
	}
}

void UVGBossSkillComponent::ResetCooldown_Q()
{
	ActiveStateTags.RemoveTag(VigilantBoss::SkillCooldown_Q);
	UE_LOG(LogTemp, Warning, TEXT("[VGBossSkill - Server] Q 스킬 다시 사용 가능"));
}

void UVGBossSkillComponent::ResetCooldown_E()
{
	ActiveStateTags.RemoveTag(VigilantBoss::SkillCooldown_E);
	UE_LOG(LogTemp, Warning, TEXT("[VGBossSkill - Server] E 스킬 다시 사용 가능"));
}

void UVGBossSkillComponent::OnSkillMontageEnded(class UAnimMontage* Montage, bool bInterrupted)
{
	// [Fix] GetOwner() null 체크 추가 — 컴포넌트가 Owner보다 오래 살아남을 수 있음
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}
	
	if (Owner->HasAuthority())
	{
		ActiveStateTags.RemoveTag(VigilantBoss::Casting);
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}
