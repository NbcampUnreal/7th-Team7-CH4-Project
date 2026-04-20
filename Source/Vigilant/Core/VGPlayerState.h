#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "Interface/VGPlayerInfoInterface.h"
#include "VGPlayerState.generated.h"


UCLASS()
class VIGILANT_API AVGPlayerState : 
public APlayerState,
public IVGPlayerInfoInterface
{
	GENERATED_BODY()
	
public:
	
	AVGPlayerState();
	
	// 플레이어 상태 태그
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Vigilant|Tags")
	FGameplayTagContainer PlayerStatusTags;
	// 플레이어 직업만 담아둘 태그 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vigilant|Role")
	FGameplayTag SecretRoleTag;
	// 태그 변경 헬퍼 함수(더하기)
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Tags")
	void AddPlayerTag(const FGameplayTag& TagToAdd);
	// 태그 변경 헬퍼 함수(빼기)
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Tags")
	void RemovePlayerTag(const FGameplayTag& TagToRemove);
	// 태그 변경 헬퍼 함수(매칭하기)
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Tags")
	bool HasPlayerTag(const FGameplayTag& TagToCheck) const;
	// 직업 확인 헬퍼 함수
	UFUNCTION(BlueprintCallable, Category = "Vigilant|Role")
	bool IsRole(const FGameplayTag& RoleTagToCheck) const;
	
	// 플레이어 이름
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vigilant|PlayerInfo")
	FString VGPlayerName;
	
	void SetVGPlayerName(const FString& NewName);

	// 플레이어 입장 순서
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vigilant|PlayerInfo")
	int32 EntryIndex;
	
	//김형백-  플레이어 매시 랜덤 숫자
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vigilant|PlayerMesh")
	int32 AssignedMeshIndex = -1;
	//플레이어 인덱스 반환 인터페이스 구현 함수 
	UFUNCTION()
	virtual int32 GetPlayerIndex() const override;
	UFUNCTION()
	virtual int32 GetRandomMeshNumber() const override;
	
	UPROPERTY(Replicated,BlueprintReadOnly,Category = "Vigilant|Lobby")
	bool bIsReady = false;
	
	UFUNCTION(Client, Reliable, Category = "Vigilant|Lobby")
	void Client_ReceiveRole(FGameplayTag AssignedRoleTag);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void CopyProperties(APlayerState* PlayerState) override;
};
