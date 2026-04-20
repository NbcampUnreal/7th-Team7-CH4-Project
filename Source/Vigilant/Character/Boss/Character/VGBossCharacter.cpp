// Fill out your copyright notice in the Description page of Project Settings.


#include "VGBossCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/Boss/Component/VGBossSkillComponent.h"
#include "Character/Component/VGStatComponent.h"
#include "Common/VGGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Data/VGBossDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


AVGBossCharacter::AVGBossCharacter()
{
	// 스킬 컴포넌트 생성 및 부착
	SkillComponent = CreateDefaultSubobject<UVGBossSkillComponent>(TEXT("SkillComponent"));
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
	
	CharacterTags.AddTag(VigilantCharacter::StaggerImmune);
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
	
	AddBossMappingContext(NewController);
}

void AVGBossCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	
	AddBossMappingContext(GetController());
}

void AVGBossCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// 달리다가 S키(뒷걸음)를 누르면 브레이크
	if (MovementVector.X < -0.1f)
	{
		if (CharacterTags.HasTag(VigilantCharacter::Sprint))
		{
			PerformStopSprint(); 
			Server_StopSprint();
		}
	}
	
	else if (MovementVector.X > 0.1f)
	{
		// 유저가 아직 Shift 유지 상태고 현재 걷고 있는 상태면
		if (bWantsToSprint && !CharacterTags.HasTag(VigilantCharacter::Sprint))
		{
			// 스태미나 확인 후 다시 달리기
			if (StatComponent && StatComponent->GetCurrentStamina() >= MinStaminaToSprint)
			{
				PerformStartSprint();
				Server_StartSprint();
			}
		}
	}
	
	Super::Move(Value);
}

void AVGBossCharacter::StartSprint(const FInputActionValue& Value)
{
	// 뒤로 걷고 있는지 검사
	FVector InputVector = GetCharacterMovement()->GetLastInputVector();
	FVector ForwardDirection = FRotationMatrix(FRotator(0, GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::X);
    
	// 입력 방향이 컨트롤러 기준 뒤쪽인지 검사
	if (FVector::DotProduct(InputVector, ForwardDirection) < -0.1f)
	{
		return;
	}
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
	// 캡슐 콜리전 제거
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	// 스켈레탈 메쉬 콜리전 끄기
	if (GetMesh())
	{
		GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	}
	// 사망 몽타주 재생
	if (BossData && BossData->DeathMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(BossData->DeathMontage); 
		}
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
