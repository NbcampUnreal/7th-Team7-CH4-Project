// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/VGBossCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AVGBossCharacter::AVGBossCharacter()
{
	// 보스의 덩치를 1.5배 크게 설정 (임시)
	SetActorScale3D(FVector(1.5f, 1.5f, 1.5f));

	// 플레이어보다 느리게 이동 속도 조절 (임시)
	if (GetCharacterMovement() != nullptr)
	{
		GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	}
	
	bReplicates = true;
	
	// 기본 스탯 초기화 (임시)
	CurrentHealth = 5000.0f;
	AttackDamage = 50.0f;
}

void AVGBossCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AVGBossCharacter, CurrentHealth);
	DOREPLIFETIME(AVGBossCharacter, AttackDamage);
}
