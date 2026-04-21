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
}

void AVGMissionCounterSandbag::OnCounterHitBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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
	if (!IsValid(LastAttacker))
	{
		return;
	}
	
	// 숙임 시작 전 현재 Roll 기록
	StartRoll = MeshComponent->GetRelativeRotation().Roll;
	CounterProgress = 0.f;
 
	// LastAttacker 방향의 Yaw 계산
	FVector ToTarget = LastAttacker->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;
	TargetYaw  = ToTarget.Rotation().Yaw;
 
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("Authority [%s] StartCounter"), *GetName());
		// Countering 상태로 전환 → 클라이언트 OnRep에서 비주얼 처리
		SetCounterState(EVGSandbagCounterState::Countering);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] StartCounter"), *GetName());
	}
}

void AVGMissionCounterSandbag::UpdateCounter(float DeltaSeconds)
{
	// ── 1. 액터 Yaw 회전 ─────────────────────────────────────────────────────
	FRotator CurrentRot = GetActorRotation();
	FRotator TargetRot  = FRotator(CurrentRot.Pitch, TargetYaw, CurrentRot.Roll);
	SetActorRotation(FMath::RInterpConstantTo(CurrentRot, TargetRot, DeltaSeconds, CounterRotationSpeed));
 
	// ── 2. MeshComponent Roll 숙이기 ─────────────────────────────────────────
	CounterProgress = FMath::Clamp(
		CounterProgress + DeltaSeconds / CounterTiltDuration,
		0.f, 1.f);
 
	FRotator MeshRot = MeshComponent->GetRelativeRotation();
	MeshRot.Roll = FMath::Lerp(StartRoll, StartRoll + CounterTiltAngle, CounterProgress);
	MeshComponent->SetRelativeRotation(MeshRot);
 
	// ── 3. 숙임 완료 판정 ────────────────────────────────────────────────────
	if (CounterProgress >= 1.f)
	{
		TriggerCounterHit();
	}
}

void AVGMissionCounterSandbag::TriggerCounterHit()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("Authority [%s] TriggerCounterHit"), *GetName());
		// 충돌체 활성화
		CounterHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] TriggerCounterHit"), *GetName());
	}
	
	// Hitting 상태 → 클라이언트 OnRep에서 이펙트 처리
	SetCounterState(EVGSandbagCounterState::Hitting);
 
	// CounterHitActiveDuration 후 종료
	GetWorldTimerManager().SetTimer(
		CounterHitTimerHandle,
		this,
		&AVGMissionCounterSandbag::FinishCounter,
		CounterHitActiveDuration,
		false);
}

void AVGMissionCounterSandbag::FinishCounter()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("Authority [%s] FinishCounter"), *GetName());
		// 충돌체 비활성화
		CounterHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CounterdCharacters.Empty();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] FinishCounter"), *GetName());
	}
 
	// Mesh Roll 원래 값으로 복귀
	FRotator MeshRot = MeshComponent->GetRelativeRotation();
	MeshRot.Roll     = StartRoll;
	MeshComponent->SetRelativeRotation(MeshRot);
 
	// Returning → Idle 순으로 전환
	SetCounterState(EVGSandbagCounterState::Returning);
	SetCounterState(EVGSandbagCounterState::Idle);
	
}

void AVGMissionCounterSandbag::SetCounterState(EVGSandbagCounterState NewState)
{
	// 서버 전용
	if (!HasAuthority())
	{
		return;
	}
	CounterState = NewState;
 
	// UE는 서버 자신의 ReplicatedUsing 콜백을 자동 호출하지 않으므로 직접 호출
	OnRep_CounterState();
}

void AVGMissionCounterSandbag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CounterState == EVGSandbagCounterState::Countering)
	{
		UpdateCounter(DeltaTime);
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

