#include "Character/Component/VGCombatComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "Data/VGWeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "VGEquipmentComponent.h"
#include "VGStatComponent.h"
#include "Equipment/VGWeapon.h"

UVGCombatComponent::UVGCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UVGCombatComponent::SetActiveCombatData(UVGWeaponDataAsset* NewData)
{
	// [Fix] GetOwner() null 체크 추가 — 컴포넌트 해제 직후 호출 시 크래시 방지
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		ActiveCombatData = NewData;
	}
}

void UVGCombatComponent::Server_SetActiveCombatData_Implementation(UVGWeaponDataAsset* NewData)
{
	SetActiveCombatData(NewData);
}

void UVGCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UVGCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UVGCombatComponent, ActiveCombatData);
	DOREPLIFETIME(UVGCombatComponent, CurrentCombatTags);
}

void UVGCombatComponent::OnRep_ActiveCombatData(UVGWeaponDataAsset* OldData)
{
	// TODO: 클라이언트 측에서 실행될 비주얼 관련 로직 작성 (e.g. UI 업데이트)
}

UVGWeaponDataAsset* UVGCombatComponent::GetCurrentCombatData() const
{
	return ActiveCombatData ? ActiveCombatData : DefaultCombatData;
}

// ---------------------------------------------------------
// INPUT & COMBO BUFFERING
// ---------------------------------------------------------

void UVGCombatComponent::TryLightAttack()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (bCanChainCombo)
	{
		if (!bHasBufferedAttack)
		{
			bHasBufferedAttack = true;
			bIsBufferedAttackHeavy = false;
		}
		return;
	}

	// [Fix] GetMesh/GetAnimInstance null 체크 추가 — 메시 미설정 시 크래시 방지
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (Data && AnimInst && AnimInst->Montage_IsPlaying(Data->LightAttackMontage))
	{
		return;
	}

	CurrentComboIndex = 0;
	PerformAttack(false);

	// [Fix] Listen server host는 HasAuthority()==true이므로 Server RPC를 건너뜀
	//       → 다른 클라이언트에 Multicast가 전송되지 않아 공격 애니메이션이 보이지 않는 문제
	if (OwnerCharacter->IsLocallyControlled())
	{
		if (OwnerCharacter->HasAuthority())
		{
			BroadcastAttackMontage(false);
		}
		else
		{
			Server_TryAttack(false, CurrentComboIndex);
		}
	}
}

void UVGCombatComponent::TryHeavyAttack()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (bCanChainCombo)
	{
		if (!bHasBufferedAttack)
		{
			bHasBufferedAttack = true;
			bIsBufferedAttackHeavy = true;
		}
		return;
	}

	// [Fix] GetMesh/GetAnimInstance null 체크 추가
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (Data && AnimInst && AnimInst->Montage_IsPlaying(Data->HeavyAttackMontage))
	{
		return;
	}

	CurrentComboIndex = 0;
	PerformAttack(true);

	// [Fix] Listen server host Multicast 누락 수정 (TryLightAttack과 동일)
	if (OwnerCharacter->IsLocallyControlled())
	{
		if (OwnerCharacter->HasAuthority())
		{
			BroadcastAttackMontage(true);
		}
		else
		{
			Server_TryAttack(true, CurrentComboIndex);
		}
	}
}

void UVGCombatComponent::PerformAttack(bool bIsHeavy)
{
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	UAnimMontage* MontageToPlay = bIsHeavy ? Data->HeavyAttackMontage : Data->LightAttackMontage;
	if (!MontageToPlay)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		FString SectionPrefix = bIsHeavy ? TEXT("Heavy") : TEXT("Light");
		FName SectionName = FName(*FString::Printf(TEXT("%s%d"), *SectionPrefix, CurrentComboIndex + 1));
		OwnerCharacter->PlayAnimMontage(MontageToPlay, Data->AttackSpeed, SectionName);
	}
}

// ---------------------------------------------------------
// ANIM NOTIFY COMBO HOOKS
// ---------------------------------------------------------

void UVGCombatComponent::OnComboWindowOpened()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	bCanChainCombo = true;
	bHasBufferedAttack = false;
}

void UVGCombatComponent::OnComboWindowClosed()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	bCanChainCombo = false;

	if (bHasBufferedAttack)
	{
		CurrentComboIndex++;
		bHasBufferedAttack = false;

		PerformAttack(bIsBufferedAttackHeavy);

		// [Fix] Listen server host 콤보 연계 시에도 Multicast 전송 필요
		if (OwnerPawn->HasAuthority())
		{
			BroadcastAttackMontage(bIsBufferedAttackHeavy);
		}
		else
		{
			Server_TryAttack(bIsBufferedAttackHeavy, CurrentComboIndex);
		}
	}
	else
	{
		CurrentComboIndex = 0;
	}
}

// ---------------------------------------------------------
// NETWORKING RPCS
// ---------------------------------------------------------


void UVGCombatComponent::Server_TryAttack_Implementation(bool bIsHeavy, int32 ExpectedComboIndex)
{
	CurrentComboIndex = ExpectedComboIndex;
	PerformAttack(bIsHeavy);
	BroadcastAttackMontage(bIsHeavy);
}

// [Fix] Listen server host용 + Server_TryAttack 공용 Multicast 헬퍼
void UVGCombatComponent::BroadcastAttackMontage(bool bIsHeavy)
{
	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (!Data)
	{
		return;
	}
	UAnimMontage* MontageToPlay = bIsHeavy ? Data->HeavyAttackMontage : Data->LightAttackMontage;
	FString SectionPrefix = bIsHeavy ? TEXT("Heavy") : TEXT("Light");
	FName SectionName = FName(*FString::Printf(TEXT("%s%d"), *SectionPrefix, CurrentComboIndex + 1));
	Multicast_PlayAttackMontage(MontageToPlay, SectionName, Data->AttackSpeed);
}

bool UVGCombatComponent::Server_TryAttack_Validate(bool bIsHeavy, int32 ExpectedComboIndex)
{
	return true;
}

void UVGCombatComponent::Client_CancelAttackPrediction_Implementation()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerCharacter->StopAnimMontage();
	}
}


void UVGCombatComponent::Multicast_PlayAttackMontage_Implementation(UAnimMontage* MontageToPlay, FName SectionName,
                                                                    float PlayRate)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	if (OwnerPawn->IsLocallyControlled() || (OwnerPawn->HasAuthority() && IsNetMode(NM_DedicatedServer)))
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerPawn);
	if (OwnerCharacter)
	{
		OwnerCharacter->PlayAnimMontage(MontageToPlay, PlayRate, SectionName);
	}
}

// ---------------------------------------------------------
// HIT DETECTION
// ---------------------------------------------------------


// [Fix] TraceMesh 결정 로직을 StartMeleeTrace/TickMeleeTrace에서 중복 사용 — 헬퍼로 추출
UMeshComponent* UVGCombatComponent::GetTraceMesh() const
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return nullptr;
	}

	if (UVGEquipmentComponent* EquipComp = OwnerCharacter->FindComponentByClass<UVGEquipmentComponent>())
	{
		if (AVGWeapon* EquippedWeapon = Cast<AVGWeapon>(EquipComp->RightHandItem))
		{
			if (UMeshComponent* WeaponMesh = EquippedWeapon->GetWeaponMesh())
			{
				return WeaponMesh;
			}
		}
	}

	return OwnerCharacter->GetMesh();
}

void UVGCombatComponent::StartMeleeTrace()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	HitActorsThisSwing.Empty();
	PreviousSocketLocations.Empty();

	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	UMeshComponent* TraceMesh = GetTraceMesh();
	if (!TraceMesh)
	{
		return;
	}

	for (const FName& SocketName : Data->HitboxSocketNames)
	{
		FVector StartLoc = TraceMesh->GetSocketLocation(SocketName);
		PreviousSocketLocations.Add(SocketName, StartLoc);
	}
}

void UVGCombatComponent::TickMeleeTrace()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	UVGWeaponDataAsset* Data = GetCurrentCombatData();
	if (!Data)
	{
		return;
	}

	UMeshComponent* TraceMesh = GetTraceMesh();
	if (!TraceMesh)
	{
		return;
	}

	float AttackRadius = 20.0f;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	// QueryParams.bTraceComplex = true;

	for (const FName& SocketName : Data->HitboxSocketNames)
	{
		if (!PreviousSocketLocations.Contains(SocketName))
		{
			continue;
		}

		FVector PreviousLoc = PreviousSocketLocations[SocketName];
		FVector CurrentLoc = TraceMesh->GetSocketLocation(SocketName);

		TArray<FHitResult> HitResults;

		bool bHit = GetWorld()->SweepMultiByChannel(
			HitResults,
			PreviousLoc,
			CurrentLoc,
			FQuat::Identity,
			ECC_Pawn,
			Sphere,
			QueryParams
		);
		
		// --- Debug ---
		FVector TraceVector = CurrentLoc - PreviousLoc;
		float TraceLength = TraceVector.Size();
		if (TraceLength > KINDA_SMALL_NUMBER)
		{
			FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();
			DrawDebugCapsule(
				GetWorld(),
				PreviousLoc + (TraceVector * 0.5f),
				(TraceLength * 0.5f) + AttackRadius,
				AttackRadius,
				CapsuleRot,
				bHit ? FColor::Green : FColor::Red,
				false,
				2.0f
			);
		}
		// ---
		
		if (bHit)
		{
			for (const FHitResult& HitResult : HitResults)
			{
				AActor* HitActor = HitResult.GetActor();
				if (HitActor && !HitActorsThisSwing.Contains(HitActor))
				{
					HitActorsThisSwing.Add(HitActor);
					Server_ProcessHit(HitActor);
				}
			}
		}

		PreviousSocketLocations[SocketName] = CurrentLoc;
	}
}

void UVGCombatComponent::StopMeleeTrace()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	HitActorsThisSwing.Empty();
}


void UVGCombatComponent::Server_ProcessHit_Implementation(AActor* HitActor)
{
	AActor* Owner = GetOwner();
	UVGWeaponDataAsset* Data = GetCurrentCombatData();

	if (!Owner || !Data || !HitActor)
	{
		return;
	}

	// [Fix] 하드코딩 300.0f → 에디터 설정 가능한 MaxHitValidationDistance 사용
	float Distance = FVector::Distance(Owner->GetActorLocation(), HitActor->GetActorLocation());
	if (Distance <= MaxHitValidationDistance)
	{
		UVGStatComponent* StatComp = HitActor->FindComponentByClass<UVGStatComponent>();
		if (StatComp)
		{
			StatComp->ApplyDamage(Data->BaseDamage);

			UE_LOG(LogTemp, Warning, TEXT("[Server] Validated Client Hit on: %s, Applying %f Damage! Current HP: %f"),
			       *HitActor->GetName(), Data->BaseDamage, StatComp->GetCurrentHP());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] Hit rejected — distance %.1f > max %.1f"),
		       Distance, MaxHitValidationDistance);
	}
}

// [Fix] 항상 true → 기본 유효성 검증 추가 (자기 자신 공격 방지, null 체크)
bool UVGCombatComponent::Server_ProcessHit_Validate(AActor* HitActor)
{
	if (!HitActor)
	{
		return false;
	}
	if (HitActor == GetOwner())
	{
		return false;
	}
	return true;
}
