// Fill out your copyright notice in the Description page of Project Settings.


#include "VGBossCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/Boss/Component/VGBossSkillComponent.h"
#include "Data/VGBossDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


AVGBossCharacter::AVGBossCharacter()
{
	JumpMaxCount = 0;
	// 스킬 컴포넌트 생성 및 부착
	SkillComponent = CreateDefaultSubobject<UVGBossSkillComponent>(TEXT("SkillComponent"));
}

void AVGBossCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 게임이 시작될 때, 데이터 에셋이 연결되어 있다면 변수에 값 세팅
	if (BossData != nullptr)
	{
		CurrentHealth = BossData->BaseHealth;
		
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = BossData->BossNormalSpeed;
		}
	}
}

void AVGBossCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		// Q 버튼을 누르면 Input_SkillQ 실행
		if (SkillAction_Q)
		{
			EnhancedInputComponent->BindAction(SkillAction_Q, ETriggerEvent::Triggered, this, &AVGBossCharacter::Input_SkillQ);
		}
        
		// E 버튼을 누르면 Input_SkillE 실행
		if (SkillAction_E)
		{
			EnhancedInputComponent->BindAction(SkillAction_E, ETriggerEvent::Triggered, this, &AVGBossCharacter::Input_SkillE);
		}
	}
}

void AVGBossCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	APlayerController* PlayerController = Cast<APlayerController>(NewController);
	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem && BossMappingContext)
		{
			// 기존 조작법을 밀어내고 보스 조작법을 최우선(Priority 1)으로 덮어씌웁니다.
			Subsystem->AddMappingContext(BossMappingContext, 1);
		}
	}
}

void AVGBossCharacter::Input_SkillQ(const FInputActionValue& Value)
{
	if (SkillComponent)
	{
		SkillComponent->ExecuteSkill_Q();
	}
}

void AVGBossCharacter::Input_SkillE(const FInputActionValue& Value)
{
	if (SkillComponent)
	{
		SkillComponent->ExecuteSkill_E();
	}
}

void AVGBossCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AVGBossCharacter, CurrentHealth);
}

void AVGBossCharacter::InitializeBossStats(float InCalculatedHealth, float InCalculatedDamage)
{
	// 스탯 적용은 서버에서만 권한을 가짐
	if (HasAuthority())
	{
		CurrentHealth = InCalculatedHealth;
		
		UE_LOG(LogTemp, Log, TEXT("보스 스탯 - 체력: %f, 공격력: %f"), CurrentHealth, InCalculatedDamage);
	}
}
