// Fill out your copyright notice in the Description page of Project Settings.


#include "VGBossCharacter.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "Character/Component/VGCombatComponent.h"
#include "Data/VGBossDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AVGBossCharacter::AVGBossCharacter()
{
	// 보스의 덩치를 1.5배 크게 설정 (임시)
	// SetActorScale3D(FVector(1.5f, 1.5f, 1.5f));

	bReplicates = true;
	JumpMaxCount = 0;
}

void AVGBossCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 게임이 시작될 때, 데이터 에셋이 연결되어 있다면 변수에 값 세팅
	if (BossData != nullptr)
	{
		CurrentHealth = BossData->BaseHealth;
		AttackDamage = BossData->BaseDamage;
		AttackRadius = BossData->AttackRadius;
		AttackMontage = BossData->AttackMontage;
		NormalSpeed = BossData->BossNormalSpeed;
		SprintSpeed = BossData->BossSprintSpeed;
		
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	}
}

void AVGBossCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AVGBossCharacter, CurrentHealth);
	DOREPLIFETIME(AVGBossCharacter, AttackDamage);
}

void AVGBossCharacter::InitializeBossStats(float InCalculatedHealth, float InCalculatedDamage)
{
	// 스탯 적용은 서버에서만 권한을 가짐
	if (HasAuthority())
	{
		CurrentHealth = InCalculatedHealth;
		AttackDamage = InCalculatedDamage;
		
		UE_LOG(LogTemp, Log, TEXT("보스 스탯 - 체력: %f, 공격력: %f"), CurrentHealth, AttackDamage);
	}
}
