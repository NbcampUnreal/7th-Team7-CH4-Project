// Fill out your copyright notice in the Description page of Project Settings.


#include "VGBossCharacter.h"

#include "Data/VGBossDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AVGBossCharacter::AVGBossCharacter()
{
	// 보스의 덩치를 1.5배 크게 설정 (임시)
	SetActorScale3D(FVector(1.5f, 1.5f, 1.5f));

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

void AVGBossCharacter::Server_PerformAttack_Implementation()
{
	// 데미지 계산은 서버에서만 진행
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("서버: 보스 광역 공격 실행"));

		// 광역 데미지 적용을 위한 무시 목록 (자기 자신은 데미지를 안 받음)
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);

		// 언리얼 내장 함수를 활용한 구형(Radial) 데미지 적용
		UGameplayStatics::ApplyRadialDamage(
			this,                  // WorldContextObject
			AttackDamage,          // 데미지 량 (데이터 에셋에서 가져온 값)
			GetActorLocation(),    // 공격 중심점 (보스 위치)
			AttackRadius,          // 공격 반경 (데이터 에셋에서 가져온 값)
			nullptr,               // 데미지 타입 클래스 (기본값)
			IgnoreActors,          // 무시할 액터 목록
			this,                  // 데미지 유발자 (보스)
			GetController(),       // 공격자 컨트롤러
			true                   // 장애물을 관통해서 데미지를 줄 것인지 여부
		);

		// 데미지 처리가 끝났으니, 모든 플레이어의 화면에 공격 이펙트를 재생하라고 지시
		NetMulticast_PlayAttackEffects();
	}
}

void AVGBossCharacter::NetMulticast_PlayAttackEffects_Implementation()
{
	// 시각적 디버깅
	DrawDebugSphere(GetWorld(), GetActorLocation(), AttackRadius, 32, FColor::Red, false, 2.0f);

	// 데이터 에셋에서 할당받은 몽타주가 있다면 애니메이션 재생
	if (AttackMontage != nullptr)
	{
		PlayAnimMontage(AttackMontage);
	}
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
