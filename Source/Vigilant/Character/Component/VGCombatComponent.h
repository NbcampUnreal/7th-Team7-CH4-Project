#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "VGCombatComponent.generated.h"

class UVGWeaponDataAsset;
class UInputAction;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VIGILANT_API UVGCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVGCombatComponent();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetActiveCombatData(UVGWeaponDataAsset* NewData);
	
	// --- Inputs ---
	void TryLightAttack();
	void TryHeavyAttack();
	
	// --- Combo Anim Notify Hooks ---
	UFUNCTION(BlueprintCallable, Category = "Combat|Animation")
	void OnComboWindowOpened();
	UFUNCTION(BlueprintCallable, Category = "Combat|Animation")
	void OnComboWindowClosed();
	
	// --- Hit Detection Anim Notify Hooks ---
	UFUNCTION(BlueprintCallable, Category = "Combat|HitDetection")
	void StartMeleeTrace();
	UFUNCTION(BlueprintCallable, Category = "Combat|HitDetection")
	void TickMeleeTrace();
	UFUNCTION(BlueprintCallable, Category = "Combat|HitDetection")
	void StopMeleeTrace();
	
	// --- 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Combat")
	TObjectPtr<UInputAction> LightAttackAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Combat")
	TObjectPtr<UInputAction> HeavyAttackAction;
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void PerformAttack(bool bIsHeavy);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryAttack(bool bIsHeavy, int32 ExpectedComboIndex);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ProcessHit(AActor* HitActor);
	
	UFUNCTION(Client, Reliable)
	void Client_CancelAttackPrediction();
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayAttackMontage(UAnimMontage* MontageToPlay, FName SectionName, float PlayRate);
	
	UFUNCTION()
	void OnRep_ActiveCombatData(UVGWeaponDataAsset* OldData);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Defaults")
	TObjectPtr<UVGWeaponDataAsset> DefaultCombatData;
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ActiveCombatData)
	TObjectPtr<UVGWeaponDataAsset> ActiveCombatData;
	
	UVGWeaponDataAsset* GetCurrentCombatData() const;
	
	// Combat State
	UPROPERTY(Replicated)
	FGameplayTagContainer CurrentCombatTags;
	
	int32 CurrentComboIndex = 0;
	bool bCanChainCombo = false;
	bool bHasBufferedAttack = false;
	bool bIsBufferedAttackHeavy = false;
	
	// Hit Detection State
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> HitActorsThisSwing;

};
