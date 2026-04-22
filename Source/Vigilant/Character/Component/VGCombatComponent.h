#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "VGCombatComponent.generated.h"

class UVGStatComponent;
class UVGAttackExecution;
class UVGShieldDataAsset;
class UVGWeaponDataAsset;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuardStateChanged, bool, bIsGuarding);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VIGILANT_API UVGCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVGCombatComponent();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetActiveCombatData(UVGWeaponDataAsset* NewData, UMeshComponent* NewTraceMesh);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetActiveShieldData(UVGShieldDataAsset* NewData);
	
	// --- Inputs ---
	void TryLightAttack();
	void TryHeavyAttack();
	void TryStartBlock();
	void TryStopBlock();
	
	// --- Combo Anim Notify Hooks ---
	UFUNCTION(BlueprintCallable, Category = "Combat|Animation")
	void OnComboWindowOpened();
	UFUNCTION(BlueprintCallable, Category = "Combat|Animation")
	void OnComboWindowClosed();
	
	// --- Anim Notify Hooks ---
	UFUNCTION(BlueprintCallable, Category = "Combat|HitDetection")
	void StartAttackExecution();
	UFUNCTION(BlueprintCallable, Category = "Combat|HitDetection")
	void TickAttackExecution();
	UFUNCTION(BlueprintCallable, Category = "Combat|HitDetection")
	void StopAttackExecution();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Combat")
	TObjectPtr<UInputAction> LightAttackAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Combat")
	TObjectPtr<UInputAction> HeavyAttackAction;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnGuardStateChanged OnGuardStateChanged;
	
public:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ProcessHit(AActor* HitActor);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnProjectile(TSubclassOf<AActor> ProjectileClass, const FVector& SpawnLocation, const FRotator& SpawnRotation);
	
	UVGWeaponDataAsset* GetCurrentCombatData() const;
	UVGShieldDataAsset* GetCurrentShieldData() const;
	UMeshComponent* GetActiveTraceMesh() const;
	
	// (이용호 추가) 데미지 배율 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetDamageMultiplier(float NewMultiplier);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void PerformAttack(bool bIsHeavy);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryAttack(bool bIsHeavy, int32 ExpectedComboIndex);

	UFUNCTION(Client, Reliable)
	void Client_CancelAttackPrediction();
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayAttackMontage(UAnimMontage* MontageToPlay, FName SectionName, float PlayRate);
	
	UFUNCTION()
	void OnRep_ActiveCombatData(UVGWeaponDataAsset* OldData);
	
	UFUNCTION()
	void OnRep_ActiveShieldData(UVGShieldDataAsset* OldData);
	
	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	// --- Block ---
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartBlock();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StopBlock();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayBlockMontage(UAnimMontage* MontageToPlay);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopBlockMontage(UAnimMontage* MontageToStop);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Defaults")
	TObjectPtr<UVGWeaponDataAsset> DefaultCombatData;
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ActiveCombatData)
	TObjectPtr<UVGWeaponDataAsset> ActiveCombatData;
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ActiveShieldData)
	TObjectPtr<UVGShieldDataAsset> ActiveShieldData;
	
	TWeakObjectPtr<UMeshComponent> ActiveTraceMesh;
	
	// Combat State
	UPROPERTY(Replicated)
	FGameplayTagContainer CurrentCombatTags;
	
	UPROPERTY(Transient)
	TObjectPtr<UVGAttackExecution> CurrentExecution;
	
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveAttackMontage;
	
	TWeakObjectPtr<UVGStatComponent> CachedStatComponent;
	
	int32 CurrentComboIndex = 0;
	bool bCanChainCombo = false;
	bool bHasBufferedAttack = false;
	bool bIsBufferedAttackHeavy = false;
	bool bIsCurrentAttackHeavy = false;
	
	// (이용호 추가) 기본 데미지 배율
	float DamageMultiplier = 1.0f;
	
private:
	void InstantiateExecutionObject();
	void SetCombatRotationMode(bool bIsAttacking);
};
