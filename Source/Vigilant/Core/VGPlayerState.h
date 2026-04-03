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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vigilant|Tags")
	FGameplayTagContainer PlayerTags;
	
};
