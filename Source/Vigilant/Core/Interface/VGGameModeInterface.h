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
};
