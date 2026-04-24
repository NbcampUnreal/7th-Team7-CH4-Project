#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "CharacterInterface/VGCharacterGameplayTagEditor.h"
#include "GameFramework/Character.h"
#include "Interaction/VGInteractable.h"
#include "Data/VGCharacterDataAsset.h"

#include "VGCharacterBase.generated.h"


class UVGLockOnComponent;
class UInputAction;
class UCameraComponent;
class UVGCombatComponent;
class USpringArmComponent;
class UVGStatComponent;
class UVGHiddenPocketComponent;

struct FInputActionValue;

UCLASS()
class VIGILANT_API AVGCharacterBase : 
public ACharacter,
public IVGCharacterGameplayTagEditor,
public IGameplayTagAssetInterface,
public IVGInteractable
{
	GENERATED_BODY()

	// Components
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Vigilant|Data")
	TObjectPtr<UVGCharacterDataAsset> CharacterDataAsset;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "GameplayTags", meta = (AllowPrivateAccess = "true"), 
		ReplicatedUsing=OnRep_CharacterTags, meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer CharacterTags;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
  
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVGCombatComponent> CombatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVGStatComponent> StatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVGLockOnComponent> LockOnComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Pocket", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVGHiddenPocketComponent> HiddenPocketComponent;
	
	
	// Camera Settings
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float DefaultCameraDistance = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float MinCameraDistance = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float MaxCameraDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraZoomSpeed = 50.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraZoomInterpSpeed = 10.0f;
	
	float TargetCameraDistance;

	// Walk & Sprint Speed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float NormalSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 900.0f;

#pragma region Interfaces Func
public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual void AddGameplayTag(FGameplayTag TagToAdd) override;
	virtual void RemoveGameplayTag(FGameplayTag TagToRemove) override;
#pragma endregion 김형백
	
	// Functions
public:
	AVGCharacterBase();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> SprintAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> LockOnAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> CameraZoomAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> HiddenPocketAction;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Components|Combat")
	FORCEINLINE UVGCombatComponent* GetCombatComponent() const { return CombatComponent; }
	
	UFUNCTION(BlueprintCallable, Category = "Components|Stat")
	FORCEINLINE UVGStatComponent* GetStatComponent() const { return StatComponent; }
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	//매쉬 결정
	void ApplyPlayerMesh();
	
	// Input Handlers
	virtual void Move(const FInputActionValue& Value);
	void StartJump(const FInputActionValue& Value);
	void StopJump(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void CameraZoom(const FInputActionValue& Value);
	void LightAttack(const FInputActionValue& Value);
	void HeavyAttack(const FInputActionValue& Value);
	void HiddenPocketToggle(const FInputActionValue& Value);
	
	// State Check
	bool CanMove() const;
	bool CanAttack() const;
	bool CanSprint() const;
	
	//캐릭터 회전 설정
	void SetCharacterRotationState(bool bIsLockedOn);
#pragma region 스프린트 관련
	//입력 바인딩
	virtual void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);
	//실제 구현
	void PerformStartSprint();
	void PerformStopSprint();
	//서버 RPC
	UFUNCTION(Server, Reliable)
	void Server_StartSprint();
	UFUNCTION(Server, Reliable)
	void Server_StopSprint();
	//스태미나 소모량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
	float SprintStaminaCostPerSecond = 10; //달리기 초당 스태미나 소모량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
	float MinStaminaToSprint = 20; //달리기에 필요한 최소 스태미나
	
	UFUNCTION()
	void HandleSprintStamina(float CurrentStamina, float Max);
	bool bWantsToSprint = false;
#pragma endregion
	
	UFUNCTION()
	void HandleLockOnTargetChanged(AActor* NewTarget);
	void LockOn(const FInputActionValue& Value);
	UPROPERTY(EditAnywhere, Category = "LockOn|Camera")
	FVector LockOnSocketOffset = FVector(0.f, 50.f, 200.f); 
	UPROPERTY(EditAnywhere, Category = "LockOn|Camera")
	FVector DefaultSocketOffset = FVector(0.f, 25.f, 100.f);
	UFUNCTION(Server, Reliable)
	void Server_SetLockOnTag(bool bIsLockedOn);
	
	UFUNCTION()
	virtual void OnRep_CharacterTags(); // 캐릭터태그 변화시 부를 콜백(내용없음)
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION(Server, Reliable)
	void ServerRPCSetSprinting(bool bIsSprinting);
	
public:
	// (이용호 추가) 플레이어간 상호작용 호출했을 때 받을 함수
	void NotifyPlayerInteraction(class AVGCharacterBase* TargetPlayer);
	
	UFUNCTION(Client, Reliable)
	void Client_ForceRotation(FRotator NewRotation, bool bKeepInputLocked = false);
	
	// 현재 해당 동작이 가능한 페이즈인지 확인용 함수
	bool IsCombatActionAllowed() const;
	bool IsInteractionAllowed(AActor* Target = nullptr) const;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor, const FTransform& InteractTransform) override;
	
protected:
	// (이용호 추가) 스탯컴포넌트의 OnDead 델리게이트와 연결용 함수
	UFUNCTION()
	virtual void HandleDeath(AController* Killer);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathRagdoll();
public:
	// --- 디버프(슬로우) 관련 함수 --- (하상빈 추가)
	void ApplySlow(float SlowMultiplier, float Duration);

protected:
	// 슬로우 해제 함수
	void ClearSlow();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetSpeedMultiplier(float NewMultiplier);

	FTimerHandle SlowTimerHandle;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float CurrentSpeedMultiplier = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Feedback")
	TSubclassOf<UCameraShakeBase> TakeDamageCameraShake;

#pragma region Stagger & Knockback
	virtual void ApplyStagger(FVector PushDirection, float KnockbackForce);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayStaggerVisual();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> StaggerMontage;
#pragma endregion Stagger & Knockback
};
