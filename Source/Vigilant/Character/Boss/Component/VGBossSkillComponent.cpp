// Fill out your copyright notice in the Description page of Project Settings.


#include "VGBossSkillComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Common/VGGameplayTags.h"
#include "Data/VGBossDataAsset.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UVGBossSkillComponent::UVGBossSkillComponent()
{
	SetIsReplicatedByDefault(true);
}

void UVGBossSkillComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UVGBossSkillComponent::ExecuteRoarAoE()
{
	if (!BossDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("[VGBossSkillComponent] BossDataAsset이 할당되지 않았음"));
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    
	// 클라이언트(시전자) 화면에서만 타격 판정을 진행
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	FVector CenterLocation = OwnerCharacter->GetActorLocation();
	FCollisionShape Sphere = FCollisionShape::MakeSphere(BossDataAsset->RoarRadius); 
    
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	TArray<FOverlapResult> OverlapResults;
    
	bool bHit = GetWorld()->OverlapMultiByChannel(
	   OverlapResults,
	   CenterLocation,
	   FQuat::Identity,
	   ECC_Pawn,
	   Sphere,
	   QueryParams
	);

	// 디버그용 구체 그리기
	DrawDebugSphere(GetWorld(), CenterLocation, BossDataAsset->RoarRadius, 32, bHit ? FColor::Green : FColor::Red, false, 2.0f);

	if (bHit)
	{
		TArray<AActor*> ActorsToHit;
       
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			AActor* HitActor = Overlap.GetActor();
          
			if (HitActor && !ActorsToHit.Contains(HitActor))
			{
				ActorsToHit.Add(HitActor);
			}
		}

		if (ActorsToHit.Num() > 0)
		{
			Server_ProcessAoEHits(ActorsToHit);
		}
	}
}

void UVGBossSkillComponent::Server_ProcessAoEHits_Implementation(const TArray<AActor*>& HitActors)
{
	if (!BossDataAsset) return;
	
	AActor* Owner = GetOwner();
	if (!Owner) return;

	for (AActor* HitActor : HitActors)
	{
		if (!HitActor) continue;

		// 서버단 거리 검증
		float Distance = FVector::Distance(Owner->GetActorLocation(), HitActor->GetActorLocation());
		float MaxAllowedDistance = BossDataAsset->RoarRadius + 150.0f;

		if (Distance <= MaxAllowedDistance)
		{
			UGameplayStatics::ApplyDamage(
			   HitActor,
			   BossDataAsset->RoarBaseDamage,
			   Owner->GetInstigatorController(),
			   Owner,
			   UDamageType::StaticClass()
			);
		}
	}
}

bool UVGBossSkillComponent::Server_ProcessAoEHits_Validate(const TArray<AActor*>& HitActors)
{
	return true;
}

void UVGBossSkillComponent::ExecuteSkill_Q()
{
	Server_ExecuteSkill_Q();
}

void UVGBossSkillComponent::Server_ExecuteSkill_Q_Implementation()
{
	if (!BossDataAsset) return;
	// [서버] 태그 및 쿨타임 검사
	if (ActiveStateTags.HasTag(VigilantBoss::Casting) || ActiveStateTags.HasTag(VigilantBoss::SkillCooldown_Q)) 
	{
		return;
	}
	// [서버] 쿨타임 및 시전 상태 돌입
	ActiveStateTags.AddTag(VigilantBoss::Casting);
	ActiveStateTags.AddTag(VigilantBoss::SkillCooldown_Q);
    
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Q, this, &UVGBossSkillComponent::ResetCooldown_Q, BossDataAsset->CooldownTime_Q, false);

	UE_LOG(LogTemp, Warning, TEXT("[VGBossSkill - Server] Q 스킬 시전 - 쿨타임 %f초"), BossDataAsset->CooldownTime_Q);
	// [서버 -> 모두에게] 모션 재생 지시
	Multicast_ExecuteSkill_Q();
}

void UVGBossSkillComponent::Multicast_ExecuteSkill_Q_Implementation()
{
	if (!BossDataAsset) return;
	// [모든 클라이언트 + 서버] 몽타주 재생 및 이동 제한
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && BossDataAsset->SkillMontage_Q)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);

		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(BossDataAsset->SkillMontage_Q);
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &UVGBossSkillComponent::OnSkillMontageEnded);
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, BossDataAsset->SkillMontage_Q);
		}
	}
}

void UVGBossSkillComponent::ExecuteSkill_E()
{
	Server_ExecuteSkill_E();
}

void UVGBossSkillComponent::Server_ExecuteSkill_E_Implementation()
{
	if (!BossDataAsset) return;
	// [서버] 태그 및 쿨타임 검사
	if (ActiveStateTags.HasTag(VigilantBoss::Casting) || ActiveStateTags.HasTag(VigilantBoss::SkillCooldown_E)) 
	{
		return;
	}
	// [서버] 쿨타임 및 시전 상태 돌입
	ActiveStateTags.AddTag(VigilantBoss::Casting);
	ActiveStateTags.AddTag(VigilantBoss::SkillCooldown_E);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_E, this, &UVGBossSkillComponent::ResetCooldown_E, BossDataAsset->CooldownTime_E, false);

	UE_LOG(LogTemp, Warning, TEXT("[VGBossSkill - Server] E 스킬 시전 - 쿨타임 %f초"), BossDataAsset->CooldownTime_E);
	// [서버 -> 모두에게] 모션 재생 지시
	Multicast_ExecuteSkill_E();
}

void UVGBossSkillComponent::Multicast_ExecuteSkill_E_Implementation()
{
	// [모든 클라이언트 + 서버] 몽타주 재생 및 이동 제한
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && BossDataAsset->SkillMontage_E)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);

		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(BossDataAsset->SkillMontage_E);
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &UVGBossSkillComponent::OnSkillMontageEnded);
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, BossDataAsset->SkillMontage_E);
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
	// [서버]에서만 상태 태그를 관리하도록 처리
	if (GetOwner()->HasAuthority())
	{
		ActiveStateTags.RemoveTag(VigilantBoss::Casting);
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}
