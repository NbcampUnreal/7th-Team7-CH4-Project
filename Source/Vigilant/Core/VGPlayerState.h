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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vigilant|Tags")
	FGameplayTagContainer PlayerTags;
	// 태그 변경 헬퍼 함수(더하기)
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Tags")
	void AddPlayerTag(const FGameplayTag& TagToAdd);
	// 태그 변경 헬퍼 함수(빼기)
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Tags")
	void RemovePlayerTag(const FGameplayTag& TagToRemove);
	// 태그 변경 헬퍼 함수(매칭하기)
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Tags")
	bool HasPlayerTag(const FGameplayTag& TagToCheck) const;
	
	UPROPERTY(Replicated,BlueprintReadOnly,Category = "Vigilant|Lobby")
	bool bIsReady = false;
	
	UFUNCTION(Client, Reliable, Category = "Vigilant|Lobby")
	void Client_ReceiveRole(FGameplayTag AssignedRoleTag);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
