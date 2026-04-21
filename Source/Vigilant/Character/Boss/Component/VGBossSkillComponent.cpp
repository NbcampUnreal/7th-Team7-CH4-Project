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
#include "Character/Boss/DamageType/VGDamageType_Slow.h"
#include "NiagaraFunctionLibrary.h"

UVGBossSkillComponent::UVGBossSkillComponent()
{
	SetIsReplicatedByDefault(true);
}

void UVGBossSkillComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UVGBossSkillComponent::ExecuteRoarSlow()
{
	if (!BossDataAsset)
	{
		return;
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}
	
	// 서버에서만 피격 판정 진행
	if (OwnerCharacter->HasAuthority())
	{
		FVector CenterLocation = OwnerCharacter->GetActorLocation();
		FCollisionShape Sphere = FCollisionShape::MakeSphere(BossDataAsset->RoarRadius); 
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(OwnerCharacter);

		TArray<FOverlapResult> OverlapResults;
		bool bHit = GetWorld()->OverlapMultiByChannel(OverlapResults, CenterLocation, FQuat::Identity, ECC_Pawn, Sphere, QueryParams);

		if (bHit)
		{
			// 한 번의 포효에 똑같은 대상이 여러 번 맞는 것을 방지
			TSet<AActor*> DamagedActors;

			for (const FOverlapResult& Overlap : OverlapResults)
			{
				AActor* HitActor = Overlap.GetActor();
             
				if (HitActor && HitActor->IsA<APawn>() && !DamagedActors.Contains(HitActor))
				{
					DamagedActors.Add(HitActor);

					UGameplayStatics::ApplyDamage(
						HitActor, 
						1.0f,
						OwnerCharacter->GetController(), 
						OwnerCharacter, 
						BossDataAsset->RoarDamageTypeClass
					);
				}
			}
		}
	}
}

void UVGBossSkillComponent::ExecuteSkill_Q()
{
	Server_ExecuteSkill_Q();
}

void UVGBossSkillComponent::Server_ExecuteSkill_Q_Implementation()
{
	if (!BossDataAsset)
	{
		return;
	}
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
	if (!BossDataAsset)
	{
		return;
	}
	// [모든 클라이언트 + 서버] 몽타주 재생 및 이동 제한
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && BossDataAsset->SkillMontage_Q)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);

		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(BossDataAsset->SkillMontage_Q);
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &UVGBossSkillComponent::OnSkillMontageEnded);
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, BossDataAsset->SkillMontage_Q);
		}
	}
}

void UVGBossSkillComponent::ExecuteLeapImpact()
{
	if (!BossDataAsset)
    {
       return;
    }
    
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
       return;
    }
	FVector ImpactLocation = OwnerCharacter->GetMesh()->GetSocketLocation(FName("HammerImpactSocket"));
	ImpactLocation.Z = OwnerCharacter->GetActorLocation().Z - 90.0f; 

    if (SkillENiagaraEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), 
            SkillENiagaraEffect, 
            ImpactLocation, 
            OwnerCharacter->GetActorRotation()
        );
    }
	
	if (SmashCameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), SmashCameraShake, ImpactLocation, 0.0f, 1500.0f);
	}
	
    // 데미지 판정 로직
    if (OwnerCharacter->HasAuthority())
    {
       FCollisionQueryParams QueryParams;
       QueryParams.AddIgnoredActor(OwnerCharacter);
       
       TArray<FOverlapResult> OverlapResults;
       
       bool bHit = GetWorld()->OverlapMultiByChannel(
          OverlapResults, 
          ImpactLocation,
          FQuat::Identity,
          ECC_Pawn,
          FCollisionShape::MakeSphere(BossDataAsset->LeapRadius),
          QueryParams
       );

       DrawDebugSphere(GetWorld(), ImpactLocation, BossDataAsset->LeapRadius, 32, FColor::Blue, false, 2.0f);

       if (bHit)
       {
          TSet<AActor*> DamagedActors;
          for (const FOverlapResult& Result : OverlapResults)
          {
             AActor* HitActor = Result.GetActor();
             
             if (HitActor && HitActor->IsA<APawn>() && !DamagedActors.Contains(HitActor))
             {
	             DamagedActors.Add(HitActor);

             	UGameplayStatics::ApplyDamage(
					 HitActor, 
					 BossDataAsset->LeapDamage,
					 OwnerCharacter->GetController(), 
					 OwnerCharacter, 
					 UDamageType::StaticClass()
				 );
             	
             	Multicast_PlayHitEffect(HitActor);
             }
          }
       }
    }
}

void UVGBossSkillComponent::Multicast_PlayHitEffect_Implementation(AActor* TargetActor)
{
	if (HitNiagaraEffect && TargetActor)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			HitNiagaraEffect,
			TargetActor->GetRootComponent(),
			NAME_None,
			FVector(0.f, 0.f, 0.f),
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
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
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
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