#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "Character/VGCharacterBase.h"
#include "Character/CharacterInterface/VGCharacterGameplayTagEditor.h"
#include "Character/Component/VGEquipmentComponent.h"
#include "Interaction/VGInteractable.h"
#include "VGCitizenCharacter.generated.h"

class UVGEquipmentDataAsset;
class UVGEquipmentComponent;
class UInputAction;
struct FInputActionValue;

UCLASS()
class VIGILANT_API AVGCitizenCharacter :
	public AVGCharacterBase
{
	GENERATED_BODY()

public:
	AVGCitizenCharacter();
	void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UVGEquipmentComponent> EquipmentComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent; 
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// 상호작용 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;
	// 아이템 버리기 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DropAction;
	// 슬롯 선택(1, 2) 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SlotSelectAction;
	//구르기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DodgeAction;
	// 가드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* BlockAction;
	//헤이!
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* HeyAction;
	// 캐릭터의 이동 상태(걷기, 낙하 등)가 변할 때마다 엔진이 호출해 주는 함수
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	// 상호작용 실행 함수
	void Interact();
	// 버리기 실행 함수
	void DropItem();
	// 슬롯 선택 실행 함수
	void SelectSlot(const FInputActionValue& Value);

	//바인딩 함수
	UFUNCTION()
	void HandleInteractFound(const FString& InfoText, const FVector& TargetLocation, bool bShow);
	
	UFUNCTION()
	void HandleItemEquipped(EVGEquipmentSlot Slot, UVGEquipmentDataAsset* EquipmentData, UMeshComponent* EquippedMesh);
	
	UFUNCTION()
	void HandlePocketItemStashed(UVGEquipmentDataAsset* EquipmentData, UMeshComponent* EquippedMesh);

	UFUNCTION()
	void HandlePocketItemDropped();
	
	
	//base의 무브 함수 재정의
	virtual void Move(const FInputActionValue& Value) override;

	void StartBlock(const FInputActionValue& Value);
	void StopBlock(const FInputActionValue& Value);

	void Dodge();
	
	// State Check
	bool CanDodge() const;
	bool CanInteract() const;
	bool CanBlock() const;

	UFUNCTION(Server, Reliable)
	void Server_Dodge(FVector Direction);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Dodge();
	void PerformDodgeAction(const FVector& Direction);


	void Hey();
	UFUNCTION(Server, Reliable) // 실행 자체는 확실히 서버에 전달되어야 하므로 Reliable
	void Server_PlayHeySound();
	UFUNCTION(NetMulticast, Unreliable) // 사운드 재생 명령은 Unreliable 권장
	void Multicast_PlayHeySound(int32 SoundIndex);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Force")
	float DodgeForce = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Force")
	float DodgeZForce = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge|Force")
	float ModifyFriction;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dodge|Force")
	float OriginalFriction; // 기본값 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> DodgeAnimation;

	UFUNCTION()
	void OnMontageCompleted(UAnimMontage* Montage, bool bWasCancelled = false);


	UFUNCTION()
	void HandleItemDropped(EVGEquipmentSlot Slot);

	UFUNCTION()
	void CheckGuardBreakOnStaminaChanged(float CurrentStamina, float MaxStamina);
	
	UFUNCTION()
	void ApplyGuardStaminaCost(bool bIsGuarding);
	
public: // 하상빈 추가
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyDebuffMaterial(float Duration);

	UFUNCTION()
	void RemoveDebuffMaterial();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debuff")
	class UMaterialInterface* RoarDebuffMaterial;

private:
	FTimerHandle DebuffMaterialTimerHandle;
};