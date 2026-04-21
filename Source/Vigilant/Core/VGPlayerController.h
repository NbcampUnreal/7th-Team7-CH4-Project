#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interface/VGChatReciveInterface.h"
#include "Interface/VGUIControllerInterface.h"
#include "VGPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class VIGILANT_API AVGPlayerController : 
public APlayerController,
public IVGChatReciveInterface,
public IVGUIControllerInterface

{
	GENERATED_BODY()
	
	// Input


	// functions
public:
	
	AVGPlayerController();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Vigilant|Lobby")
	void Server_SetReady(bool bReady);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Vigilant|Lobby")
	void Server_SetName(const FString& NewName);
	
	virtual void BeginPlay() override;
	virtual void AcknowledgePossession(class APawn* P) override; //LifeCycle함수 - 빙의 후 (클라이언트전용) 
	virtual void OnPossess(class APawn* P) override;
	
	UFUNCTION()
	void OnChatMessageReceived(const FString& Message);
	
	// 투표 전송용
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SubmitVote(int32 TargetIndex);
	
	/** 채팅 전송을 위한 RPC*/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendChatMessage(const FString& ChatText);
	UFUNCTION(Client, Reliable)
	void Client_ReceiveChatMessage(const FString& Message);
	// 게임모드 -> 플레이어 컨트롤러를 위한 인터페이스 구현함수. Wrapping ClientRPC 
	virtual void ReceiveChatMessage(const FString& Message) override;
	//UI전달을 위한 인터페이스 구현함수
	virtual void ShowInteractUI(const FString& InteractText, const FVector& TargetLocation, bool bShow) override;
	
	virtual void UpdateEquipIconUI(int32 SlotIndex, UTexture2D* Icon) override;
	virtual void ClearEquipIconUI(int32 SlotIndex) override;
	
	UFUNCTION(Client, Reliable)
	void Client_SetInputToGame();
	
	
	
protected:
	// 페이즈 변경 시 UI 처리용 위한 함수
	UFUNCTION()
	void HandleUIByPhase(FGameplayTag NewPhaseTag);
	UFUNCTION()
	void HandleTimeReduced(float NewEndTime);
	
	// GameState 대기용
	FTimerHandle BindTimerHandle;
	// GameState가 있는지 확인하고 바인딩을 시도할 함수
	void TryBindGameState();
};
