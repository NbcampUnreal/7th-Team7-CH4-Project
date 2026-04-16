#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "CharacterInterface/VGCharacterGameplayTagEditor.h"
#include "GameFramework/Character.h"
#include "Interaction/VGInteractable.h"
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
	/**
	 * 
	 */
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float DefaultCameraDistance = 400.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float MinCameraDistance = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float MaxCameraDistance = 800.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraZoomSpeed = 50.0f;

	// Walk & Sprint Speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float NormalSpeed = 600.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
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
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;
	
	// Input Handlers
	virtual void Move(const FInputActionValue& Value);
	void StartJump(const FInputActionValue& Value);
	void StopJump(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	
	void CameraZoom(const FInputActionValue& Value);
	void LightAttack(const FInputActionValue& Value);
	void HeavyAttack(const FInputActionValue& Value);
	void HiddenPocketToggle(const FInputActionValue& Value);
	
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
	void Client_ForceRotation(FRotator NewRotation);
	
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor, const FTransform& InteractTransform) override;
	

	
#pragma region Stagger & Knockback
	virtual void ApplyStagger(FVector PushDirection, float KnockbackForce);
	

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayStaggerVisual();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> StaggerMontage;
#pragma endregion Stagger & Knockback
};
