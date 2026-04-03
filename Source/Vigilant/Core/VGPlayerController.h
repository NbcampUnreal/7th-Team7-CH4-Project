#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VGPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class VIGILANT_API AVGPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	// Input


	// functions
public:
	AVGPlayerController();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	virtual void BeginPlay() override;
	virtual void AcknowledgePossession(class APawn* P) override; //LifeCycle함수 - 빙의 후 (클라이언트전용) 
};
