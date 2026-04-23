// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VGStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, NewHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, NewStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDead, AController*, Killer);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VIGILANT_API UVGStatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVGStatComponent();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public: // [1] 스탯 조작 함수 (GameMode 및 캐릭터 제어용)
    UFUNCTION(BlueprintCallable, Category = "VG|Stat")
    void ApplyDamageToStat(float DamageAmount, AController* Instigator);

    UFUNCTION(BlueprintCallable, Category = "VG|Stat")
    void RecoverHP(float RecoverAmount);

    UFUNCTION(BlueprintCallable, Category = "VG|Stat")
    void ConsumeStamina(float ConsumeAmount);

    UFUNCTION(BlueprintCallable, Category = "VG|Stat")
    void RecoverStamina(float RecoverAmount);
    
    UFUNCTION(BlueprintCallable, Category = "VG|Stat")
    void StartContinuousConsumeStamina(float ConsumeAmountPerSecond);
    
    UFUNCTION(BlueprintCallable, Category = "VG|Stat")
    void StopContinuousConsumeStamina();
    
    UFUNCTION(BlueprintCallable, Category = "VG|Stat")
    void ResetStats();
    
public: // [2] 데이터 접근 (Getter)
    UFUNCTION(BlueprintPure, Category = "VG|Stat") 
    float GetCurrentHP() const { return CurrentHP; }
    
    UFUNCTION(BlueprintPure, Category = "VG|Stat") 
    float GetMaxHP() const { return MaxHP; }
    
    UFUNCTION(BlueprintPure, Category = "VG|Stat") 
    float GetCurrentStamina() const { return CurrentStamina; }
    
    UFUNCTION(BlueprintPure, Category = "VG|Stat") 
    float GetMaxStamina() const { return MaxStamina; }
    
    UFUNCTION(BlueprintPure, Category = "VG|Stat") 
    bool GetIsAlive() const { return bIsAlive; }

public: // [3] 이벤트 델리게이트 (HUD 및 애니메이션 바인딩용)
    UPROPERTY(BlueprintAssignable, Category = "VG|Stat|Event")
    FOnHPChanged OnHPChanged;

    UPROPERTY(BlueprintAssignable, Category = "VG|Stat|Event")
    FOnStaminaChanged OnStaminaChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "VG|Stat|Event")
    FOnDead OnDead;
    
public:
    // 보스 캐릭터 동적 스탯 초기화 함수 추가 [하상빈] 
    UFUNCTION(BlueprintCallable, Category = "VG|Stat")
    void InitStat(float InMaxHP, float InMaxStamina);

protected: // [4] 내부 동작 함수
    virtual void BeginPlay() override;
    
    void RegenerateStamina();
    void StartStaminaRegenTimer();
    void UseStaminaTick();
    
    // [5] 변수
private: /* 체력 */
    UPROPERTY(ReplicatedUsing = OnRep_MaxHP, EditDefaultsOnly, Category = "VG|Stat|Health")
    float MaxHP = 100.f;
    
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, VisibleAnywhere, Category = "VG|Stat|Health")
    float CurrentHP; 
    
    UPROPERTY(Transient, Replicated)
    AController* LastInstigator; // 게임 플레이 전용 변수
    
private: /* 스태미나 */
    UPROPERTY(EditDefaultsOnly, Category = "VG|Stat|Stamina")
    float MaxStamina = 100.f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina, VisibleAnywhere, Category = "VG|Stat|Stamina")
    float CurrentStamina; 
    
    UPROPERTY(EditDefaultsOnly, Category = "VG|Stat|Stamina")
    float StaminaRegenRate = 15.0f; // 초당 회복량

    UPROPERTY(EditDefaultsOnly, Category = "VG|Stat|Stamina")
    float StaminaRegenDelay = 1.5f; // 소모 후 회복 시작까지 대기 시간

    UPROPERTY(EditDefaultsOnly, Category = "VG|Stat|Stamina")
    float StaminaRegenInterval = 0.1f; // 회복 틱 간격 (0.1초마다)
    
    UPROPERTY(EditDefaultsOnly, Category = "VG|Stat|Stamina")
    float StaminaConsumeInterval = 0.1f; // 스테미나 소모 틱 간격
    
    float ContinuousConsumeRate = 0.f; //내부 사용 변수
    
private: /* 타이머 핸들 */
    FTimerHandle StaminaRegenTimerHandle;
    FTimerHandle StaminaContinuousConsumeTimerHandle;
 
private: /* 상태 */
    UPROPERTY(ReplicatedUsing = OnRep_bIsAlive, VisibleAnywhere, Category = "VG|Stat|State")
    bool bIsAlive = true; 

private: // [6] 네트워크 동기화 콜백 함수 (OnRep)
    UFUNCTION() void OnRep_MaxHP(); 
    UFUNCTION() void OnRep_CurrentHP(float OldHP);
    UFUNCTION() void OnRep_CurrentStamina();
    UFUNCTION() void OnRep_bIsAlive();
};
