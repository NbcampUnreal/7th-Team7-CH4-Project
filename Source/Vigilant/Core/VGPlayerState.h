#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "VGPlayerState.generated.h"


UCLASS()
class VIGILANT_API AVGPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	
	AVGPlayerState();
	
	// 플레이어 상태 태그
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vigilant|Tags")
	FGameplayTagContainer PlayerStatusTags;
	// 태그 변경 헬퍼 함수(더하기)
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Tags")
	void AddPlayerTag(const FGameplayTag& TagToAdd);
	// 태그 변경 헬퍼 함수(빼기)
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Tags")
	void RemovePlayerTag(const FGameplayTag& TagToRemove);
	// 태그 변경 헬퍼 함수(매칭하기)
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Tags")
	bool HasPlayerTag(const FGameplayTag& TagToCheck) const;
	
	// 플레이어 이름
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vigilant|PlayerInfo")
	FString VGPlayerName;

	// 플레이어 입장 순서
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vigilant|PlayerInfo")
	int32 EntryIndex;
	
	UPROPERTY(Replicated,BlueprintReadOnly,Category = "Vigilant|Lobby")
	bool bIsReady = false;
	
	UFUNCTION(Client, Reliable, Category = "Vigilant|Lobby")
	void Client_ReceiveRole(FGameplayTag AssignedRoleTag);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
