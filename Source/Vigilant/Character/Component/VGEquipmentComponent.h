#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "VGEquipmentComponent.generated.h"

class AVGCharacterBase;
class AVGEquippableActor;

// 장착 슬롯 구분 (왼손, 오른손, 양손)
UENUM(BlueprintType)
enum class EVGEquipmentSlot : uint8
{
	None UMETA(DisplayName = "None"),
	LeftHand UMETA(DisplayName = "Left Hand"),
	RightHand UMETA(DisplayName = "Right Hand"),
	BothHands UMETA(DisplayName = "Both Hands")
};

// 아이템 장착 시 어느 슬롯에 어떤 아이템이 들어왔는지 방송
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemEquippedSignature, EVGEquipmentSlot, Slot, UVGEquipmentDataAsset*, EquipmentData, UMeshComponent*, EquippedMesh);
// 아이템 해제 시 어느 슬롯이 비었는지 방송
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDroppedSignature, EVGEquipmentSlot, Slot);
// 어떤 슬롯이 활성화 되었는지 방송, 활성화 될때마다 캐스트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentSlotChangedSignature, EVGEquipmentSlot, NewActiveSlot);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VIGILANT_API UVGEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVGEquipmentComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
public:
	// 양손 인벤토리 변수
	// 왼손 (방패 또는 미션 아이템)
	UPROPERTY(ReplicatedUsing = OnRep_LefthandItem, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Slots")
	TObjectPtr<AVGEquippableActor> LeftHandItem;

	// 오른손 (무기 또는 미션 아이템)
	UPROPERTY(ReplicatedUsing = OnRep_RighthandItem, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Slots")
	TObjectPtr<AVGEquippableActor> RightHandItem;
	
	// 클라이언트가 서버에게 아이템 장착을 요청하는 함수
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Equipment|Action")
	void Server_EquipItem(AVGEquippableActor* ItemToEquip);
	
	// 클라이언트가 서버에게 아이템 버리기를 요청하는 함수
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Equipment|Action")
	void Server_DropItem(EVGEquipmentSlot SlotToDrop);
	
	// UI 바인드용 변수 
	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnItemEquippedSignature OnItemEquipped;
	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnItemDroppedSignature OnItemDropped;
	// UI(블루프린트)에서 이벤트로 끌어다 쓸 수 있는 델리게이트 변수
	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnEquipmentSlotChangedSignature OnEquipmentSlotChanged;
	
	
	// 현재 활성화된 슬롯을 기억할 변수 (기본값: 오른손)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	EVGEquipmentSlot ActiveEquipmentSlot = EVGEquipmentSlot::RightHand;
	
	// 서버 전용 상호작용 함수
	UFUNCTION(Server, Reliable)
	void Server_InteractWithActor(AActor* TargetActor, AActor* Interactor, const FTransform& InteractTransform);
	
	void Interact();
	void DropItem();
	void SelectSlot(float SlotNumber);
	
protected:
	UFUNCTION()
	void OnRep_LefthandItem(AVGEquippableActor* OldItem);
	
	UFUNCTION()
	void OnRep_RighthandItem(AVGEquippableActor* OldItem);
	
	void HandleItemAttachment(AVGEquippableActor* Item, FName SocketName, bool bIsEquipping);
	
	bool TryEquipToRightHand(AVGEquippableActor* ItemToEquip);
	bool TryEquipToLeftHand(AVGEquippableActor* ItemToEquip);
	bool TryEquipToEitherHand(AVGEquippableActor* ItemToEquip);
	bool TryEquipToBothHands(AVGEquippableActor* ItemToEquip);
	
private:
    
	// 상호작용 대상 탐색 타이머
	FTimerHandle TimerHandle_UpdateInteractable;
	float InteractionCheckInterval = 0.1f; // 0.1초마다 내 주변 스캔

	// 현재 하이라이트된(가장 가까운) 상호작용 타겟
	UPROPERTY()
	AActor* CurrentInteractableTarget = nullptr;

	void UpdateInteractableTarget();
	void SetHighlight(AActor* TargetActor, bool bHighlight);
};
