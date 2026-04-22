// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VGMissionSandbag.h"
#include "VGMissionCounterSandbag.generated.h"

class AVGCharacterBase;
class UBoxComponent;

// 반격 단계 표현용 열거형
UENUM()
enum class EVGSandbagCounterState : uint8
{
	Idle       UMETA(DisplayName = "Idle"),       // 대기
	Countering UMETA(DisplayName = "Countering"), // 회전 + 숙임 진행 중
	Hitting    UMETA(DisplayName = "Hitting"),    // 충돌 판정 활성화 중
	Returning  UMETA(DisplayName = "Returning"),  // 원래 자세로 복귀 중
};

UCLASS()
class VIGILANT_API AVGMissionCounterSandbag : public AVGMissionSandbag
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVGMissionCounterSandbag();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 카운터 전용 충돌체에 충돌시의 동작
	UFUNCTION(BlueprintNativeEvent)
	void OnCounterHitBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor*              OtherActor,
		UPrimitiveComponent* OtherComp,
		int32                OtherBodyIndex,
		bool                 bFromSweep,
		const FHitResult&    SweepResult);
	
	// --- 반격 로직 함수들 ---
	void StartCounter();		// 반격 시작(Server Only)
	void UpdateCounter(float DeltaSeconds);		// 숙이기(Both)
	void TriggerCounterHit();		// 판정 시작 (Server Only)
	void UpdateCounterReturning(float DeltaTime);	// 복귀(Both)
	void FinishCounter();		// 대기 상태로 복귀 (Server Only)
	
	UFUNCTION()
	void OnCounterHitTimerExpired();   // 타이머 종료 후 Returning으로 전환용
	
	void SetCounterState(EVGSandbagCounterState NewState);
		
	UFUNCTION()
	void OnRep_CounterState();
	
	UFUNCTION()
	void OnRep_HitCount();
	
	virtual void OnDead(AController* LastInstigator) override;
	virtual void OnHPChanged(float NewHP, float MaxHP) override;

	
	UFUNCTION(BlueprintImplementableEvent)
	void OnChangeCounterState(EVGSandbagCounterState CurrentState);
	
public:
	virtual void Tick(float DeltaTime) override;
	
protected:
	
protected:
	// 카운터 전용 충돌체
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Counter")
	TObjectPtr<UBoxComponent> CounterHitBox;
	
	// About Counter
	// 반격 상태 표현
	UPROPERTY(ReplicatedUsing = OnRep_CounterState, VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Counter")
	EVGSandbagCounterState CounterState = EVGSandbagCounterState::Idle;
	
	// 반격 대상을 바라보기 위해 회전하는 속도
	UPROPERTY(EditAnywhere, Category = "Mission|Counter")
	float CounterRotationSpeed = 360.f;
	
	// 반격의 충돌 유지 시간
	UPROPERTY(EditAnywhere, Category = "Mission|Counter")
	float CounterHitActiveDuration = 0.2f;
	
	// 반격 시 넉백 강도
	UPROPERTY(EditAnywhere, Category = "Mission|Counter")
	float CounterKnockbackStrength = 800.f;
	
	// 반격까지의 피격 횟수
	UPROPERTY(EditAnywhere, Category = "Mission|Counter")
	int32 CounterTriggerInterval = 1;
 
	// 현재 픽겨 횟수
	UPROPERTY(ReplicatedUsing = OnRep_HitCount, VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Counter")
	int32 HitCount = 0;
	
	// 숙이는 양
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Counter")
	float CounterTiltAngle;
	
	// 숙이는 데 걸리는 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Counter")
	float CounterTiltDuration;
	
	// 숙이기 진행 정도
	float CounterProgress;
	
	// 반격 대상을 바라보는 Yaw
	UPROPERTY(Replicated)
	float TargetYaw = 0.f;
	
	// 숙이기 시작한 각도
	UPROPERTY(Replicated)
	float StartRoll = 0.f;
	
	// 카운터 충돌체 비활성을 위한 타이머
	FTimerHandle CounterHitTimerHandle;
	
	TArray<TObjectPtr<AVGCharacterBase>> CounterdCharacters;
};
