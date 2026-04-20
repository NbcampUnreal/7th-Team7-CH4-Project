#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VGMissionSandbag.generated.h"

class AVGCharacterBase;
class UVGStatComponent;
// 막타 플레이어 정보를 미션에 전달하는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
FOnSandbagDefeated, AVGCharacterBase*, LastAttacker);

// 샌드백 리셋을 외부에 알리는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSandbagReseted);

// 피격 시 외부에 알리는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSandbagHitted);

UCLASS()
class VIGILANT_API AVGMissionSandbag : public AActor
{
	GENERATED_BODY()

public:
	AVGMissionSandbag();
	// 데미지를 입힌 플레이어를 추적하기 위해 외부에서 직접 호출
	UFUNCTION(BlueprintCallable)
	void RegisterAttacker(AVGCharacterBase* Attacker);
	
	void ResetSandbag();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator, AActor* DamageCauser) override;
private:
	UFUNCTION()
	void OnDead(AController* LastInstigator);

	UFUNCTION()
	void OnHPChanged(float NewHP, float MaxHP);
	
	UFUNCTION()
	void OnRep_CurrentHPRatio();
public:
	UPROPERTY(BlueprintAssignable)
	FOnSandbagDefeated OnSandbagDefeated;
	
	UPROPERTY(BlueprintAssignable)
	FOnSandbagHitted OnSandbagHitted;
	
	UPROPERTY(BlueprintAssignable)
	FOnSandbagReseted OnSandbagReseted;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sandbag")
	TObjectPtr<UVGStatComponent> StatComponent;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<USceneComponent> RootComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	// 마지막으로 데미지를 준 플레이어 — 서버 전용
	UPROPERTY()
	TObjectPtr<AVGCharacterBase> LastAttacker;
	
	// 마지막 피격 위치
	UPROPERTY(Transient,Replicated)
	FVector LastHitLocation;
	// 마지막 피격 노말
	UPROPERTY(Transient,Replicated)
	FVector LastHitNormal;
	
	// HP 바 표시용 — Replicated
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHPRatio, Category = "Mission|Samdbag")
	float CurrentHPRatio = 1.f;
};
