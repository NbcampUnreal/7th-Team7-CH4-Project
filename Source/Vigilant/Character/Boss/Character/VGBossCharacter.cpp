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
	// [Fix] ACharacter는 기본적으로 bReplicates = true — 중복 설정 제거
	JumpMaxCount = 0;
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
	
	// [Fix] CastChecked는 실패 시 크래시 → Cast로 변경하여 안전하게 null 처리
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
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
	
	// [Fix] PossessedBy는 서버에서만 호출됨 — Listen server host인 경우에만 여기서 IMC 등록
	AddBossMappingContext(NewController);
}

void AVGBossCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	
	// [Fix] 리모트 클라이언트는 PossessedBy가 호출되지 않음 — 클라이언트 측 경로에서 IMC 등록
	AddBossMappingContext(GetController());
}

void AVGBossCharacter::AddBossMappingContext(AController* InController)
{
	APlayerController* PlayerController = Cast<APlayerController>(InController);
	if (!PlayerController || !BossMappingContext)
	{
		return;
	}
	
	if (ULocalPlayer* LP = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
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
