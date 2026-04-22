#include "VGMissionCounterSandbag.h"
#include "Character/VGCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "Character/Component/VGStatComponent.h"

AVGMissionCounterSandbag::AVGMissionCounterSandbag()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	// 반격 판정 충돌체 — 평소엔 끔, TriggerCounterHit에서만 활성화
	CounterHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CounterHitBox"));
	CounterHitBox->SetupAttachment(MeshComponent);
	CounterHitBox->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CounterHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AVGMissionCounterSandbag::BeginPlay()
{
	Super::BeginPlay();
	
	// 충돌 판정은 서버에서만 진행
	if (HasAuthority())
	{
		CounterHitBox->OnComponentBeginOverlap.AddDynamic(
			this, &AVGMissionCounterSandbag::OnCounterHitBoxOverlap);
	}
}

void AVGMissionCounterSandbag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CounterState);
	DOREPLIFETIME(ThisClass, TargetYaw);
	DOREPLIFETIME(ThisClass, StartRoll);
}

void AVGMissionCounterSandbag::OnCounterHitBoxOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || OtherActor == this)
	{
		return;
	}
	
	AVGCharacterBase* HitCharacter = Cast<AVGCharacterBase>(OtherActor);
	if (!HitCharacter || CounterdCharacters.Contains(HitCharacter))
	{
		return;
	}
	
	CounterdCharacters.Add(HitCharacter);
	
	// 샌드백 앞 방향 + 살짝 위로 넉백
	// 이미 회전을 해서 샌드백 앞 방향 == 대상을 바라보는 방향
	FVector KnockbackDir = GetActorForwardVector();
	KnockbackDir.Z = 0.3f;
	KnockbackDir.Normalize();
 
	HitCharacter->LaunchCharacter(
		KnockbackDir * CounterKnockbackStrength,
		true,   // bXYOverride
		true);  // bZOverride
}

void AVGMissionCounterSandbag::StartCounter()
{
	if (!HasAuthority() || !IsValid(LastAttacker))
	{
		return;
	}
	
	// 1. 서버에서 회전 목표값 및 시작 각도 계산
	StartRoll = MeshComponent->GetRelativeRotation().Roll;
	CounterProgress = 0.f;
	
	FVector ToTarget = LastAttacker->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;
	TargetYaw  = ToTarget.Rotation().Yaw;
 
	// 2. 상태 변경 (이 시점에 클라이언트로 데이터가 전송됨)
	SetCounterState(EVGSandbagCounterState::Countering);
}

void AVGMissionCounterSandbag::UpdateCounter(float DeltaTime)
{
	// ── 1. 액터 Yaw 회전 (공격자 바라보기) ──
	FRotator CurrentRot = GetActorRotation();
	FRotator TargetRot  = FRotator(CurrentRot.Pitch, TargetYaw, CurrentRot.Roll);
	SetActorRotation(FMath::RInterpConstantTo(CurrentRot, TargetRot, DeltaTime, CounterRotationSpeed));
 
	// ── 2. MeshComponent Roll 숙이기 (CounterProgress 사용) ──
	CounterProgress = FMath::Clamp(
		CounterProgress + DeltaTime / CounterTiltDuration,
		0.f, 1.f);
 
	FRotator MeshRot = MeshComponent->GetRelativeRotation();
	MeshRot.Roll = FMath::Lerp(StartRoll, StartRoll + CounterTiltAngle, CounterProgress);
	MeshComponent->SetRelativeRotation(MeshRot);
 
	// ── 3. 숙임 완료 판정 시서버에서 다음 단계로 전이) ───
	if (HasAuthority() && CounterProgress >= 1.f)
	{
		TriggerCounterHit();
	}
}

void AVGMissionCounterSandbag::UpdateCounterReturning(float DeltaTime)
{	
	//Roll 다시 세우기 (1.0 -> 0.0)
	CounterProgress = FMath::Clamp(
			CounterProgress - DeltaTime / CounterTiltDuration,
			0.f, 1.f);
	
	FRotator MeshRot = MeshComponent->GetRelativeRotation();
	MeshRot.Roll = FMath::Lerp(StartRoll, StartRoll + CounterTiltAngle, CounterProgress);
	MeshComponent->SetRelativeRotation(MeshRot);
 
	// 복귀 완료 시 서버에서 종료 처리
	if (HasAuthority() && CounterProgress <= 0.f)
	{
		FinishCounter();
	}
}

void AVGMissionCounterSandbag::TriggerCounterHit()
{
	if (!HasAuthority())
	{
		return;
	}
	// 충돌체 활성화
	CounterHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// Hitting 상태 → 클라이언트 OnRep에서 이펙트 처리
	SetCounterState(EVGSandbagCounterState::Hitting);
 
	// CounterHitActiveDuration 후 복귀 상태로 전환
	GetWorldTimerManager().SetTimer(
		CounterHitTimerHandle,
		this,
		&AVGMissionCounterSandbag::OnCounterHitTimerExpired,
		CounterHitActiveDuration,
		false);
}

void AVGMissionCounterSandbag::FinishCounter()
{
	if (!HasAuthority())
	{
		return;
	}
	
	// Idle로 전환
	SetCounterState(EVGSandbagCounterState::Idle);
}

void AVGMissionCounterSandbag::OnCounterHitTimerExpired()
{
	if (!HasAuthority())
	{
		return;
	}
	
	// 서버: 충돌체 끄고 Returning 상태로 변경
	CounterHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CounterdCharacters.Empty();
    
	SetCounterState(EVGSandbagCounterState::Returning);
}

void AVGMissionCounterSandbag::SetCounterState(EVGSandbagCounterState NewState)
{
	if (HasAuthority())
	{
		CounterState = NewState;
		OnRep_CounterState(); // 서버도 콜백 직접 호출
	}
}

void AVGMissionCounterSandbag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	switch (CounterState)
	{
	case EVGSandbagCounterState::Countering:
		{
			UpdateCounter(DeltaTime);
		}
		break;
	case EVGSandbagCounterState::Returning:
		{
			UpdateCounterReturning(DeltaTime);
		}
		break;
	default:
		break;
	}
}

void AVGMissionCounterSandbag::OnDead(AController* LastInstigator)
{
	Super::OnDead(LastInstigator);
	
	// 반격 도중 사망 시 정리
	// SetActorTickEnabled(false);
	GetWorldTimerManager().ClearTimer(CounterHitTimerHandle);
	CounterHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetCounterState(EVGSandbagCounterState::Idle);
}

void AVGMissionCounterSandbag::OnHPChanged(float NewHP, float MaxHP)
{
	Super::OnHPChanged(NewHP, MaxHP);
	
	// Reset으로 HP가 Full이 된 상황은 무시
	if (FMath::IsNearlyEqual(NewHP,MaxHP))
	{
		return;
	}
	
	// Idle 상태일 때만 카운트 (반격 중에 맞아도 중첩 반격 없음)
	if (HasAuthority() && StatComponent->GetIsAlive()
		&& CounterState == EVGSandbagCounterState::Idle)
	{
		UE_LOG(LogTemp, Log, TEXT("Authority [%s] Hit detected"), *GetName());
		++HitCount;
		OnRep_HitCount();
	}
}

void AVGMissionCounterSandbag::OnRep_CounterState()
{
	switch (CounterState)
	{
	case EVGSandbagCounterState::Idle:
		break;
 
	case EVGSandbagCounterState::Countering:
		
		break;
 
	case EVGSandbagCounterState::Hitting:
		break;
 
	case EVGSandbagCounterState::Returning:
		break;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[%s] CounterState : %s"),
		*GetName(), *UEnum::GetValueAsString(CounterState));
	// 이펙트 관련은 BP에서 작업할 예정
	OnChangeCounterState(CounterState);
}

void AVGMissionCounterSandbag::OnRep_HitCount()
{
	if (HitCount >= CounterTriggerInterval)
	{
		if (HasAuthority())
		{
			HitCount = 0;
		}
		StartCounter();
	}
	UE_LOG(LogTemp, Log, TEXT("[%s] HitCount : %d"), *GetName(), HitCount);
}

