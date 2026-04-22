// Fill out your copyright notice in the Description page of Project Settings.


#include "VGBossCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/Boss/Component/VGBossSkillComponent.h"
#include "Character/Component/VGCombatComponent.h"
#include "Character/Component/VGStatComponent.h"
#include "Common/VGGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Data/VGBossDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Subsystem/VGUIManagerSubsystem.h"

AVGBossCharacter::AVGBossCharacter()
{
	// 스킬 컴포넌트 생성 및 부착
	SkillComponent = CreateDefaultSubobject<UVGBossSkillComponent>(TEXT("BossSkillComponent"));
}

void AVGBossCharacter::ApplyNerfAndInitStat(float NerfRate)
{
	// 서버에서만 실행됨
	if (HasAuthority() && BossData)
	{
		// 보스 체력, 스태미나 설정
		if (BossData && StatComponent)
		{
			float FinalHealth = BossData->BaseHealth * NerfRate;
			StatComponent->InitStat(FinalHealth, StatComponent->GetMaxStamina());
		}
        
		// 보스 공격력 설정
		if (CombatComponent)
		{
			CombatComponent->SetDamageMultiplier(NerfRate);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[VGBossCharacter] 보스 스탯 배율 확인용: %.2f"), NerfRate);
	}
	
}

void AVGBossCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (StatComponent)
	{
		StatComponent->OnDead.AddDynamic(this, &AVGBossCharacter::Die);
	}
	
	// 게임이 시작될 때, 데이터 에셋이 연결되어 있다면 변수에 값 세팅
	if (BossData != nullptr)
	{
		NormalSpeed = BossData->BossNormalSpeed;
		SprintSpeed = BossData->BossSprintSpeed;
		
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
		}
		
		if (StatComponent && HasAuthority())
		{
			StatComponent->InitStat(BossData->BaseHealth, StatComponent->GetMaxStamina());
		}
	}
	
	if (CombatComponent)
	{
		CombatComponent->SetActiveCombatData(CombatComponent->GetCurrentCombatData(), GetMesh());
	}
  
	CharacterTags.AddTag(VigilantCharacter::StaggerImmune);
	
	if (StatComponent)
	{
		if (APlayerController* LocalPlayerController = GetWorld()->GetFirstPlayerController())
		{
			if (ULocalPlayer* LocalPlayer = LocalPlayerController->GetLocalPlayer())
			{
				if (UVGUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UVGUIManagerSubsystem>())
				{
					// VGUIManagerSubsystem의 OnBossHealthUpdate 함수에 체력 변경 바안딩
					StatComponent->OnHPChanged.AddUniqueDynamic(UIManager, &UVGUIManagerSubsystem::OnBossHealthUpdate);
					
					// 초기화 (최대체력에 연동)
					UIManager->OnBossHealthUpdate(StatComponent->GetCurrentHP(), StatComponent->GetMaxHP());
				}
			}
		}
	}
}

void AVGBossCharacter::AddBossMappingContext(AController* InController)
{
	APlayerController* PlayerController = Cast<APlayerController>(InController);
	if (!PlayerController || !BossMappingContext) 
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(BossMappingContext, 100);
		}
	}
}

void AVGBossCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		// Q 버튼을 누르면 Input_SkillQ 실행
		if (SkillAction_Q)
		{
			EnhancedInputComponent->BindAction(SkillAction_Q, ETriggerEvent::Started, this, &AVGBossCharacter::Input_SkillQ);
		}
        
		// E 버튼을 누르면 Input_SkillE 실행
		if (SkillAction_E)
		{
			EnhancedInputComponent->BindAction(SkillAction_E, ETriggerEvent::Started, this, &AVGBossCharacter::Input_SkillE);
		}
	}
}

void AVGBossCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AddBossMappingContext(NewController);
}

void AVGBossCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	AddBossMappingContext(GetController());
}

void AVGBossCharacter::Move(const FInputActionValue& Value)
{
	Super::Move(Value);
}

void AVGBossCharacter::StartSprint(const FInputActionValue& Value)
{
	Super::StartSprint(Value);
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

float AVGBossCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	
	if (CharacterTags.HasTag(VigilantBoss::Dead))
	{
		return 0.0f;
	}
	
	float ActualDamage = 0.0f;
	if (HasAuthority()) 
	{
		// 방어력 계산 로직
		if (BossData)
		{
			float Defense = BossData->BaseDefense;
           
			// 데미지 * (100 / (100 + 방어력))
			ActualDamage = DamageAmount * (100.0f / (100.0f + Defense));
		}
		else
		{
			ActualDamage = DamageAmount;
		}
        
		// 스탯 컴포넌트에 최종 데미지 전달
		if (StatComponent && ActualDamage > 0.f)
		{
			StatComponent->ApplyDamageToStat(ActualDamage, EventInstigator);
		}
	}
	return ActualDamage;
}

void AVGBossCharacter::Die(AController* Killer)
{
	if (!HasAuthority()) return;
	
	CharacterTags.AddTag(VigilantBoss::Dead);
	
	// 캐릭터 이동 컴포넌트 비활성화
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}
	// 입력 차단
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		DisableInput(PlayerController);
	}
	Multicast_Die();
}

void AVGBossCharacter::Multicast_Die_Implementation()
{
	// 이동 및 물리 정지
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}
	
	// 캡슐 콜리전 제거
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
	
	if (BossData && BossData->DeathMontage)
	{
		PlayAnimMontage(BossData->DeathMontage);
	}
}

void AVGBossCharacter::InitializeBossStats(float InCalculatedHealth, float InCalculatedDamage)
{
	// 스탯 적용은 서버에서만 권한을 가짐
	if (HasAuthority())
	{
		if (StatComponent)
		{
			StatComponent->InitStat(InCalculatedHealth, StatComponent->GetMaxStamina());
		}
		UE_LOG(LogTemp, Log, TEXT("보스 스탯 초기화 완료 - 체력: %f, 공격력: %f"), InCalculatedHealth, InCalculatedDamage);
	}
}
