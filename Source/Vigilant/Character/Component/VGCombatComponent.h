#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "VGCombatComponent.generated.h"

class UVGAttackExecution;
class UVGWeaponDataAsset;
class UInputAction;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VIGILANT_API UVGCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVGCombatComponent();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetActiveCombatData(UVGWeaponDataAsset* NewData, UMeshComponent* NewTraceMesh);
	
	// --- Inputs ---
	void TryLightAttack();
	void TryHeavyAttack();
	
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
	
public:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ProcessHit(AActor* HitActor);
	
	UVGWeaponDataAsset* GetCurrentCombatData() const;
	UMeshComponent* GetActiveTraceMesh() const;

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
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Defaults")
	TObjectPtr<UVGWeaponDataAsset> DefaultCombatData;
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ActiveCombatData)
	TObjectPtr<UVGWeaponDataAsset> ActiveCombatData;
	
	TWeakObjectPtr<UMeshComponent> ActiveTraceMesh;
	
	// Combat State
	UPROPERTY(Replicated)
	FGameplayTagContainer CurrentCombatTags;
	
	int32 CurrentComboIndex = 0;
	bool bCanChainCombo = false;
	bool bHasBufferedAttack = false;
	bool bIsBufferedAttackHeavy = false;
	
	UPROPERTY(Transient)
	TObjectPtr<UVGAttackExecution> CurrentExecution;
	
private:
	void InstantiateExecutionObject();
};
