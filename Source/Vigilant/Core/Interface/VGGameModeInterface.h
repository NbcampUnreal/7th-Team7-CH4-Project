#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VGGameModeInterface.generated.h"


UINTERFACE()
class VIGILANT_API UVGGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

class VIGILANT_API IVGGameModeInterface
{
	GENERATED_BODY()

public:
	// 막고라 요청을 받기만할 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Vigilant|GameRule")
	void RequestDuelPhase(class AVGCharacterBase* Challenger, class AVGCharacterBase* Target);
	
	// 공격 가능한지 확인용
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Vigilant|GameRule")
	bool CanPlayerAttack(class AVGCharacterBase* Attacker);

	// 상호작용 가능한지 확인용
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Vigilant|GameRule")
	bool CanPlayerInteract(class AVGCharacterBase* Interactor, AActor* Target);

	// 데미지 받을 수 있는지 확인용
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Vigilant|GameRule")
	bool CanPlayerTakeDamage(AActor* DamageCauser, class AVGCharacterBase* Target);
	
	// 플레이어가 죽었을 때 해당 페이즈 규칙 실행용
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Vigilant|GameRule")
	void NotifyPlayerDeath(class AVGCharacterBase* Killer, class AVGCharacterBase* Victim);
};
